#ifndef LCMYSQL_BUILDER_H
#define LCMYSQL_BUILDER_H

#include <cstdint>
#include <string>
#include <istream>
#include <list>
#include <memory>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/warning.h>
#include <cppconn/metadata.h>
#include <cppconn/parameter_metadata.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/resultset_metadata.h>
#include <cppconn/statement.h>
#include <cppconn/connection.h>
#include <cppconn/datatype.h>

#include "sqldecls.h"
#include "sqlarg.h"

LC_SQL_DECL_BEGIN

template <typename R>
class SqlMapper
{
public:
    virtual R map(sql::ResultSet& rs) = 0;
};

class SqlBuilder
{
private:
    std::string _builder;
    std::list<SqlArg*> _args;

public:
    std::string sql()
    {
        return std::string(_builder);
    }
    const std::list<SqlArg*>& args() const
    {
        return _args;
    }
    SqlBuilder& operator+=(std::string sql)
    {
        _builder.append(sql);
        return *this;
    }
    SqlBuilder& arg(SqlArg* arg)
    {
        _args.push_back(arg);
        return *this;
    }

    template<typename ArgType>
    SqlBuilder& operator<<(const ArgType& arg)
    {
        return this->arg(new SqlArg(arg));
    }
};


LC_SQL_DECL_END

#endif
