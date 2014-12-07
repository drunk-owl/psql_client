#ifndef PSQL_TCP_CONN_H
#define PSQL_TCP_CONN_H

#include "psql_conn.h"
#include <boost/asio/ip/tcp.hpp>

namespace PSQL
{

class TcpConn : public Conn
{
public:
    TcpConn(boost::asio::io_service &io_service);
    ~TcpConn();

protected:
    void initSocket(int sd);
    void pollReading();
    void pollWriting();

private:
    boost::asio::ip::tcp::socket *sock;
};

}

#endif // PSQL_TCP_CONN_H
