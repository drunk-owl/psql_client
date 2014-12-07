#ifndef PG_RESULT_H
#define PG_RESULT_H

#include <postgresql/libpq-fe.h>
#include <string>

namespace PSQL
{

class Result
{
public:
    Result(PGresult *result);
    ~Result();

    int rowCount() const;
    int columnCount() const;
    bool next();
    std::string value(int col) const;

    bool isOk() const;
    std::string errorString() const;

private:
    Result(const Result&);
    Result& operator =(const Result&);

    PGresult *m_result;
    int m_cur_row;
};

}

#endif // PG_RESULT_H
