#include "psql_conn_pool.h"
#include "psql_conn.h"
#include "psql_result.h"

using namespace PSQL;

ConnPool::ConnPool(boost::asio::io_service &io_service) :
    m_max_conn_count(4),
    m_on_error(0),
    m_io_service(io_service),
    m_dd_tmr(io_service, boost::posix_time::time_duration(0,0,0))
{
}

void ConnPool::pushQuery(std::string &&query, std::vector<std::string> &&params, ResultCallback callback)
{
    QueryHolder *qh=new QueryHolder(std::move(query), std::move(params), callback);
    if(!m_idle_conns.empty())
    {
        Conn *conn=m_idle_conns.back();
        m_idle_conns.pop_back();
        m_busy_conns.insert(std::pair<Conn*, QueryHolder*>(conn, qh));
        conn->beginQuery(qh->m_query, qh->m_params, qh->m_callback);
    }
    else
    {
        m_query_queue.push_back(qh);

        if(m_busy_conns.size()+m_new_conns.size() < m_max_conn_count)
        {
            addConnection();
        }
    }
}

void ConnPool::pushQuery(std::string &&query, ResultCallback callback)
{
    pushQuery(std::move(query), std::move(std::vector<std::string>()), callback);
}

void ConnPool::addConnection()
{
    Conn *conn=new Conn(m_io_service);
    m_new_conns.push_back(conn);
    conn->beginConnect(m_host, m_port, m_dbname, m_user, m_password,
                       std::bind(&ConnPool::onConnReady, this, conn),
                       std::bind(&ConnPool::onConnError, this, conn, std::placeholders::_1));
}

void ConnPool::deleteLater(Conn *conn)
{
    m_dd_tmr.async_wait([conn](const boost::system::error_code&){delete conn;});
}

void ConnPool::onConnReady(Conn *conn)
{
    auto it=m_busy_conns.find(conn);
    if(it==m_busy_conns.end())
    {
        m_new_conns.remove(conn);
    }
    else
    {
        delete it->second;
        m_busy_conns.erase(it);
    }

    if(!m_query_queue.empty())
    {
        QueryHolder *qh=m_query_queue.front();
        m_query_queue.pop_front();
        m_busy_conns.insert(std::pair<Conn*, QueryHolder*>(conn, qh));
        conn->beginQuery(qh->m_query, qh->m_params, qh->m_callback);
    }
    else
    {
        m_idle_conns.push_back(conn);
    }
}

void ConnPool::onConnError(Conn *conn, const boost::system::error_code &err)
{
    if(m_on_error)
    {
        if(err.value()==0)
        {
            m_on_error(conn->errorString());
        }
        else
        {
            m_on_error(err.message());
        }
    }

    auto it=m_busy_conns.find(conn);
    if(it==m_busy_conns.end())
    {
        m_new_conns.remove(conn);
        deleteLater(conn);

        if(!m_query_queue.empty())
        {
            addConnection();
        }
    }
    else
    {
        m_query_queue.push_front(it->second);
        deleteLater(conn);
        m_busy_conns.erase(it);

        addConnection();
    }
}
