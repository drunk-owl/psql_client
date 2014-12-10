#include "psql_conn.h"
#include "assert.h"
#include "psql_result.h"
#include <boost/bind.hpp>

using namespace PSQL;

Conn::Conn(boost::asio::io_service &io_service) :
    m_io_service(io_service), m_conn(0)
{
}

Conn::~Conn()
{
    if(m_conn)
    {
        PQfinish(m_conn);
    }
}

void Conn::beginConnect(const std::string &host, const std::string &port, const std::string &dbname, const std::string &user, const std::string &password,
                        std::function<void()> on_ready, std::function<void(const boost::system::error_code&)> on_error)
{
    const char *const keywords[6] = {"host", "port", "dbname", "user", "password", 0};
    const char *const values[6] = {host.c_str(), port.c_str(), dbname.c_str(), user.c_str(), password.c_str(), 0};

    m_conn=PQconnectStartParams(keywords, values, 0);
    if(m_conn==0)
        throw std::bad_alloc();

    if(PQstatus(m_conn)==CONNECTION_BAD)
        throw std::string(PQerrorMessage(m_conn));

    m_on_ready=on_ready;
    m_on_error=on_error;

    m_state=Connecting;
    initSocket(PQsocket(m_conn));
    processConnecting();
}

void Conn::beginQuery(const std::string &query, const std::vector<std::string> &params, std::function<void(Result&&)> on_result)
{
    assert(m_state==Idle);

    const char *paramValues[params.size()];
    int paramLengths[params.size()];
    int i=0;
    for(const std::string& p : params)
    {
        paramValues[i]=p.c_str();
        paramLengths[i]=p.size();
        ++i;
    }

    if(PQsendQueryParams(m_conn, query.data(), params.size(), 0, paramValues, paramLengths, 0, 0)==0)
    {
        m_on_error(boost::system::error_code());
        return;
    }

    m_on_result=on_result;

    m_state=Sending;
    processSending();
}

std::string Conn::errorString() const
{
    return std::string(PQerrorMessage(m_conn));
}

void Conn::processConnecting()
{
    switch(PQconnectPoll(m_conn))
    {
    case PGRES_POLLING_FAILED:
        m_on_error(boost::system::error_code());
        return;
    case PGRES_POLLING_READING:
        pollReading();
        return;
    case PGRES_POLLING_WRITING:
        pollWriting();
        return;
    case PGRES_POLLING_OK:
        m_state=Idle;
        m_on_ready();
        return;
    default:
        break;
    }
}

void Conn::processSending()
{
    switch(PQflush(m_conn))
    {
    case -1:
        m_on_error(boost::system::error_code());
        return;
    case 0:
        m_state=Receiving;
        processReceiving();
        return;
    case 1:
        pollWriting();
        return;
    }
}

void Conn::processReceiving()
{
    if(PQconsumeInput(m_conn)==0)
    {
        m_on_error(boost::system::error_code());
        return;
    }

    if(PQisBusy(m_conn))
    {
        pollReading();
    }
    else
    {
        m_state=Idle;

        m_on_result(std::move(Result(PQgetResult(m_conn))));
        m_on_ready();
    }
}

void Conn::onReadyRead(const boost::system::error_code &error_code, std::size_t)
{
    if(error_code)
    {
        m_on_error(error_code);
        return;
    }

    switch(m_state)
    {
    case Connecting:
        processConnecting();
        return;
    case Receiving:
        processReceiving();
        return;
    default:
        assert(false);
        break;
    }
}

void Conn::onReadyWrite(const boost::system::error_code &error_code, std::size_t)
{
    if(error_code)
    {
        m_on_error(error_code);
        return;
    }

    switch(m_state)
    {
    case Connecting:
        processConnecting();
        return;
    case Sending:
        processSending();
        return;
    default:
        assert(false);
        break;
    }
}
