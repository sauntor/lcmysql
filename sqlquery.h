#ifndef LCMYSQL_QUERY_H
#define LCMYSQL_QUERY_H

#include <memory>
#include <map>
#include <cppconn/datatype.h>
#include <cppconn/resultset.h>
#include <cppconn/resultset_metadata.h>
#include <cppconn/variant.h>
#include <lcmysql/sqldecls.h>
#include <lcmysql/sqlbuilder.h>
#include <lcmysql/sqlexception.h>

LC_SQL_DECL_BEGIN

namespace internal
{
    static std::shared_ptr<sql::PreparedStatement> prepareStatement(sql::Connection& connection, SqlBuilder& builder);
}

template<typename T>
std::shared_ptr< std::list<T> >  Query(sql::Connection& connection, SqlBuilder& builder, SqlMapper<T>& mapper)
{
    std::shared_ptr<sql::PreparedStatement> stmt = internal::prepareStatement(connection, builder);
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
T QueryOne(sql::Connection& connection, SqlBuilder& builder, SqlMapper<T>& mapper) throw(NotOnlyOneException, HasNoneException)
{
    std::shared_ptr<sql::PreparedStatement> stmt = internal::prepareStatement(connection, builder);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    size_t count = rs->rowsCount();
    if (count > 1)
    {
        //std::string s = "excepted rowsCount is 1, but more than one row(" + std::to_string(count) + ") was selected";
        //throw NotOnlyOneException(builder.sql(), builder.args(), 1, count, s.c_str());
        throw NotOnlyOneException(builder.sql(), builder.args(), 1, count, "excepted rowsCount is 1, but more than one row(" + std::to_string(count) + ") was selected");
    }
    if (rs->next())
    {
        T result = mapper.map(*rs);
        return result;
    }
    else
    {
        throw NotOnlyOneException(builder.sql(), builder.args(), 1, 0, "excepted rowsCount is 1, but no row was selected");
    }
}

template<typename T>
std::shared_ptr<T>  QueryOne(sql::Connection& connection, SqlBuilder& builder, SqlMapper<T*>& mapper) throw(NotOnlyOneException, HasNoneException)
{
    std::shared_ptr<sql::PreparedStatement> stmt = internal::prepareStatement(connection, builder);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    size_t count = rs->rowsCount();
    if (count > 1)
    {
        throw NotOnlyOneException(builder.sql(), builder.args(), 1, count, "excepted rowsCount is 1, but more than one row was selected");
    }
    if (rs->next())
    {
        T* result = mapper.map(*rs);
        return std::shared_ptr<T>(result);
    }
    else
    {
        throw NotOnlyOneException(builder.sql(), builder.args(), 1, 0, "excepted rowsCount is 1, but no row was selected");
    }
}

std::shared_ptr< std::map< std::string, SqlArg* > >  QueryMap(sql::Connection& connection,
        SqlBuilder& builder, const SqlArg::Type types[]) throw(NotOnlyOneException, HasNoneException)
{
    std::shared_ptr< std::map< std::string, SqlArg* > > one(new std::map<std::string, SqlArg*>());

    std::shared_ptr<sql::PreparedStatement> stmt = internal::prepareStatement(connection, builder);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    size_t count = rs->rowsCount();
    if (count > 1)
    {
        throw NotOnlyOneException(builder.sql(), builder.args(), 1, count, "excepted rowsCount is 1, but more than one row was selected");
    }
    if (rs->next())
    {
        sql::ResultSetMetaData* metadata = rs->getMetaData();
        unsigned int columns = metadata->getColumnCount();
        for (int i = 1; i <= columns; i++)
        {
            std::string label = metadata->getColumnLabel(i).asStdString();
            switch(types[i - 1])
            {
                case SqlArg::Type::Bool:
                {
                    (* one)[label] = new SqlArg(rs->getBoolean(i));
                    break;
                }
                case SqlArg::Type::Int:
                {
                    (* one)[label] = new SqlArg(rs->getInt(i));
                    break;
                }
                case SqlArg::Type::UInt:
                {
                    (* one)[label] = new SqlArg(rs->getUInt(i));
                    break;
                }
                case SqlArg::Type::Long:
                {
                    (* one)[label] = new SqlArg(rs->getInt64(i));
                    break;
                }
                case SqlArg::Type::ULong:
                {
                    (* one)[label] = new SqlArg(rs->getUInt64(i));
                    break;
                }
                case SqlArg::Type::String:
                {
                    (* one)[label] = new SqlArg(rs->getString(i).asStdString());
                    break;
                }
                case SqlArg::Type::Blob:
                {
                    (* one)[label] = new SqlArg(rs->getBlob(i));
                    break;
                }
            }
        }
    }
    else
    {
        throw NotOnlyOneException(builder.sql(), builder.args(), 1, 0, "excepted rowsCount is 1, but no row was selected");
    }
    return one;
}



int Delete(sql::Connection& connection, SqlBuilder& builder) throw(ArgTypeException)
{
    std::shared_ptr<sql::PreparedStatement> stmt = internal::prepareStatement(connection, builder);
    return stmt->executeUpdate();
}

int Update(sql::Connection& connection, SqlBuilder& builder) throw(ArgTypeException)
{
    std::shared_ptr<sql::PreparedStatement> stmt = internal::prepareStatement(connection, builder);
    return stmt->executeUpdate();
}




// ======================================
// internal(private) implementations
// ======================================
std::shared_ptr<sql::PreparedStatement> internal::prepareStatement(sql::Connection& connection, SqlBuilder& builder)
{
    using Type = SqlArg::Type;
    std::shared_ptr<sql::PreparedStatement> stmt(connection.prepareStatement(builder.sql()));
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
            {
                throw ArgTypeException(arg->type(),
                        "bad sql arg type: " + std::to_string((int) arg->type()));
            }            
        }
    }
    return stmt;
}

LC_SQL_DECL_END

#endif
