#include "psql_tcp_conn.h"

using namespace PSQL;

TcpConn::TcpConn(boost::asio::io_service &io_service) :
    Conn(io_service),
    sock(0)
{
}

TcpConn::~TcpConn()
{
    if(sock)
    {
        delete sock;
    }
}

void TcpConn::initSocket(int sd)
{
    sock=new boost::asio::ip::tcp::socket(m_io_service, boost::asio::ip::tcp::v4(), sd);
}

void TcpConn::pollReading()
{
    sock->async_read_some(boost::asio::null_buffers(), std::bind(&TcpConn::onReadyRead, this, std::placeholders::_1, std::placeholders::_2));
}

void TcpConn::pollWriting()
{
    sock->async_write_some(boost::asio::null_buffers(), std::bind(&TcpConn::onReadyWrite, this, std::placeholders::_1, std::placeholders::_2));
}
