#ifndef LCMYSQL_EXCEPTION_H
#define LCMYSQL_EXCEPTION_H

#include <string>
#include <stdexcept>
#include "sqldecls.h"
#include "sqlarg.h"

LC_SQL_DECL_BEGIN

class QueryException : public std::runtime_error
{
public:
    QueryException(const std::string& msg) : std::runtime_error(msg) {};
    QueryException(const char* msg) : std::runtime_error(msg) {};
};

class ArgTypeException : public QueryException
{
private:
    SqlArg::Type _type;

public:
    ArgTypeException(SqlArg::Type type, const std::string& msg) : QueryException(msg)
    {
        _type = type;
    }
    ArgTypeException(SqlArg::Type type, const char* msg) : QueryException(msg)
    {
        _type = type;
    }
};

class _SqlResultCountException : public QueryException
{
private:
    const std::string& _sql;
    const std::list<SqlArg*>& _args;
    size_t _desired;
    size_t _actual;

public:
    _SqlResultCountException(const std::string& sql, const std::list<SqlArg*>& args, size_t desired, size_t actual, const char* msg)
        : QueryException(msg), _sql(sql), _args(args), _desired(desired), _actual(actual) {}
    _SqlResultCountException(const std::string& sql, const std::list<SqlArg*>& args, size_t desired, size_t actual, const std::string& msg)
        : QueryException(msg), _sql(sql), _args(args), _desired(desired), _actual(actual) {}
};

typedef _SqlResultCountException NotOnlyOneException;
typedef _SqlResultCountException HasNoneException;

LC_SQL_DECL_END

#endif
