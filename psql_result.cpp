#include "psql_result.h"

using namespace PSQL;

Result::Result(PGresult *result) :
    m_result(result),
    m_cur_row(-1)
{
}

Result::~Result()
{
    if(m_result)
    {
        PQclear(m_result);
    }
}

int Result::rowCount() const
{
    return PQntuples(m_result);
}

int Result::columnCount() const
{
    return PQnfields(m_result);
}

bool Result::next()
{
    ++m_cur_row;
    return (m_cur_row<rowCount());
}

std::string Result::value(int col) const
{
    size_t len=0;
    unsigned char* buf=PQunescapeBytea((const unsigned char*)PQgetvalue(m_result, m_cur_row, col), &len);
    std::string result((char*)buf, len);
    PQfreemem(buf);
    return result;
}

bool Result::isOk() const
{
    ExecStatusType status=PQresultStatus(m_result);
    return (status==PGRES_TUPLES_OK || status==PGRES_COMMAND_OK);
}

std::string Result::errorString() const
{
    return std::string(PQresultErrorField(m_result, PG_DIAG_MESSAGE_PRIMARY));
}
