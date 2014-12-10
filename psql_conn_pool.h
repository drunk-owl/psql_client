#ifndef PG_CONN_POOL_H
#define PG_CONN_POOL_H

#include "property.h"
#include <string>
#include <functional>
#include <vector>
#include <list>
#include <map>
#include <postgresql/libpq-fe.h>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>

namespace PSQL
{

class Result;
class Conn;

typedef std::function<void(Result&&)> ResultCallback;

class ConnPool
{
public:
    ConnPool(boost::asio::io_service &io_service);

    DECLARE_PROPERTY(std::string,host)
    DECLARE_PROPERTY(std::string,port)
    DECLARE_PROPERTY(std::string,dbname)
    DECLARE_PROPERTY(std::string,user)
    DECLARE_PROPERTY(std::string,password)
    DECLARE_PROPERTY(unsigned,max_conn_count)
    DECLARE_PROPERTY(std::function<void(const std::string&)>,on_error)

public:
    void pushQuery(std::string&& query, std::vector<std::string> &&params, ResultCallback callback);
    void pushQuery(std::string&& query, ResultCallback callback);

private:
    class QueryHolder;

    void addConnection();
    void deleteLater(Conn *conn);

    void onConnReady(Conn *conn);
    void onConnError(Conn *conn, const boost::system::error_code &err);

    boost::asio::io_service &m_io_service;
    boost::asio::deadline_timer m_dd_tmr;
    std::list<Conn*> m_new_conns;
    std::vector<Conn*> m_idle_conns;
    std::map<Conn*, QueryHolder*> m_busy_conns;
    std::list<QueryHolder*> m_query_queue;
};

class ConnPool::QueryHolder
{
public:
    QueryHolder(std::string&& query, std::vector<std::string>&& params, ResultCallback callback) :
        m_query(std::move(query)), m_params(std::move(params)), m_callback(callback) {}
    std::string m_query;
    std::vector<std::string> m_params;
    ResultCallback m_callback;
};

}

#endif // PG_CONN_POOL_H
