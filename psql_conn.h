#ifndef PG_CONN_H
#define PG_CONN_H

#include <boost/asio/io_service.hpp>
#include <postgresql/libpq-fe.h>
#include <functional>
#include <boost/system/error_code.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

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

    void beginConnect(const std::string &host, const std::string &port, const std::string &dbname, const std::string &user, const std::string &password,
                      std::function<void()> on_ready, std::function<void(const boost::system::error_code&)> on_error);
    void beginQuery(const std::string &query, const std::vector<std::string> &params, std::function<void(Result&&)> on_result);

    std::string errorString() const;

private:
    void processConnecting();
    void processSending();
    void processReceiving();

    void pollReading();
    void pollWriting();

    void onReadyRead(const boost::system::error_code &error_code, std::size_t);
    void onReadyWrite(const boost::system::error_code &error_code, std::size_t);

    boost::asio::io_service &m_io_service;
    PGconn *m_conn;
    boost::asio::posix::stream_descriptor *m_sock;
    std::function<void()> m_on_ready;
    std::function<void(const boost::system::error_code&)> m_on_error;
    std::function<void(Result&&)> m_on_result;
    State m_state;

};

}

#endif // PG_CONN_H
