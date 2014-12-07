#ifndef PG_CONN_H
#define PG_CONN_H

#include <boost/asio/io_service.hpp>
#include <postgresql/libpq-fe.h>
#include <functional>
#include <boost/system/error_code.hpp>

namespace PSQL
{

class Result;

class Conn
{
    enum State
    {
        Connecting,
        Sending,
        Receiving,
        Idle
    };

public:
    Conn(boost::asio::io_service &io_service);
    virtual ~Conn();

    void init(std::function<void(Result*)> &&on_ready, std::function<void(const boost::system::error_code&)> &&on_error);
    void beginConnect(const std::string &host, const std::string &port, const std::string &dbname, const std::string &user, const std::string &password);
    void beginQuery(const std::string &query, const std::vector<std::string> &params);

    std::string errorString() const;

protected:
    virtual void initSocket(int sd) = 0;
    virtual void pollReading() = 0;
    virtual void pollWriting() = 0;

    void onReadyRead(const boost::system::error_code &error_code, std::size_t);
    void onReadyWrite(const boost::system::error_code &error_code, std::size_t);

    boost::asio::io_service &m_io_service;

private:
    void processConnecting();
    void processSending();
    void processReceiving();

    PGconn *m_conn;
    std::function<void(Result*)> m_on_ready;
    std::function<void(const boost::system::error_code&)> m_on_error;
    State m_state;

};

}

#endif // PG_CONN_H
