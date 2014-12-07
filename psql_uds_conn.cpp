#include "psql_uds_conn.h"

using namespace PSQL;

UdsConn::UdsConn(boost::asio::io_service &io_service) :
    Conn(io_service),
    sock(0)
{
}

UdsConn::~UdsConn()
{
    if(sock)
    {
        delete sock;
    }
}

void UdsConn::initSocket(int sd)
{
    sock=new boost::asio::local::stream_protocol::socket(m_io_service, boost::asio::local::stream_protocol(), sd);
}

void UdsConn::pollReading()
{
    sock->async_read_some(boost::asio::null_buffers(), std::bind(&UdsConn::onReadyRead, this, std::placeholders::_1, std::placeholders::_2));
}

void UdsConn::pollWriting()
{
    sock->async_write_some(boost::asio::null_buffers(), std::bind(&UdsConn::onReadyWrite, this, std::placeholders::_1, std::placeholders::_2));
}
