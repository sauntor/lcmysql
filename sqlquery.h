#ifndef LCMYSQL_QUERY_H
#define LCMYSQL_QUERY_H

#include <lcmysql/sqldecls.h>
#include <lcmysql/sqlbuilder.h>

LC_SQL_DECL_BEGIN

std::shared_ptr<sql::PreparedStatement> prepareStatement(sql::Connection& connection, SqlBuilder& builder);

template<typename T>
std::shared_ptr< std::list<T> >  Query(sql::Connection& connection, SqlBuilder& builder, SqlMapper<T>& mapper)
{
    std::shared_ptr<sql::PreparedStatement> stmt = prepareStatement(connection, builder);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::list<T>* list = new std::list<T>();
    while(rs->next())
    {
        T result = mapper.map(*rs);
        list->push_back(result);
    }
    return std::shared_ptr< std::list<T> >(list);
}

template<typename T>
T QueryOne(sql::Connection& connection, SqlBuilder& builder, SqlMapper<T>& mapper)
{
    std::shared_ptr<sql::PreparedStatement> stmt = prepareStatement(connection, builder);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    size_t count = rs->rowsCount();
    if (count > 1)
    {
        throw "result rows count > 1, but single row is desired";
    }
    if (rs->next())
    {
        T result = mapper.map(*rs);
        return result;
    }
    else
    {
        throw "no row selected";
    }
}

template<typename T>
std::shared_ptr<T>  QueryOne(sql::Connection& connection, SqlBuilder& builder, SqlMapper<T*>& mapper)
{
    std::shared_ptr<sql::PreparedStatement> stmt = prepareStatement(connection, builder);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    size_t count = rs->rowsCount();
    if (count > 1)
    {
        throw "result rows count > 1, but single row is desired";
    }
    if (rs->next())
    {
        T* result = mapper.map(*rs);
        return std::shared_ptr<T>(result);
    }
    else
    {
        throw "no row selected";
    }
}

std::shared_ptr<sql::PreparedStatement> prepareStatement(sql::Connection& connection, SqlBuilder& builder)
{
    using Type = SqlArg::Type;
    std::shared_ptr<sql::PreparedStatement> stmt(connection.prepareStatement( builder.sql()));
    const std::list<SqlArg*>& args = builder.args();
    int k = 1;
    for (std::list<SqlArg*>::const_iterator i = args.cbegin(); i != args.cend(); ++i, k++)
    {
        const SqlArg* arg = *i;
        switch(arg->type())
        {
            case Type::Int:
                stmt->setInt(k, arg->asInt());
                break;
            case Type::Long:
                stmt->setInt64(k, arg->asLong());
                break;
            case Type::Bool:
                stmt->setBoolean(k, arg->asBool());
                break;
            case Type::UInt:
                stmt->setUInt(k, arg->asUInt());
                break;
            case Type::ULong:
                stmt->setUInt64(k, arg->asULong());
                break;
            case Type::Double:
                stmt->setDouble(k, arg->asDouble());
                break;
            case Type::String:
            {
                stmt->setString(k, *arg->asString());
                break;
            }
            case Type::Blob:
                stmt->setBlob(k, arg->asBlob());
                break;
            default:
                throw &"bad sql type " [ arg->type()];
        }
    }
    return stmt;
}

LC_SQL_DECL_END

#endif
