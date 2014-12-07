#ifndef PSQL_UDS_CONN_H
#define PSQL_UDS_CONN_H

#include "psql_conn.h"
#include <boost/asio/local/stream_protocol.hpp>

namespace PSQL
{

class UdsConn : public Conn
{
public:
    UdsConn(boost::asio::io_service &io_service);
    ~UdsConn();

protected:
    void initSocket(int sd);
    void pollReading();
    void pollWriting();

private:
    boost::asio::local::stream_protocol::socket *sock;
};

}

#endif // PSQL_UDS_CONN_H
