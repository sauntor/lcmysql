#ifndef LCMYSQL_QUERY_H
#define LCMYSQL_QUERY_H

#include <lcmysql/sqldecls.h>
#include <lcmysql/sqlbuilder.h>
#include <lcmysql/sqlexception.h>

LC_SQL_DECL_BEGIN

namespace internal
{
    static std::shared_ptr<sql::PreparedStatement> prepareStatement(sql::Connection& connection, SqlBuilder& builder);
    static std::string escape(std::string & str, const char* c, const char * p);
    static const char* escape(const char *str, const char* c, const char * p);
    static const char* SQL_PARAM_HOLDER = "?";
    static int _doUpdate(sql::Connection& connection, SqlBuilder& builder) throw(ArgTypeException);
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

int Delete(sql::Connection& connection, SqlBuilder& builder) throw(ArgTypeException)
{
    return internal::_doUpdate(connection, builder);
}

int Update(sql::Connection& connection, SqlBuilder& builder) throw(ArgTypeException)
{
    return internal::_doUpdate(connection, builder);
}




// ============================== internal(private) implementations ============================
static std::string internal::escape(std::string & str, const char* c, const char * p)
{
    int j = 0;
    std::stringstream s;
    for(std::string::iterator i = str.begin(); i != str.end(); i++)
    {
        j++;
        if (*i == *c)
        {
            s << p;
        }
        else
        {
            s << *i;
        }
    }
    str = s.str();
    return str;
}
static const char* internal::escape(const char *str, const char* c, const char * p)
{
    std::string ss(str);
    return escape(ss, c, p).c_str();
}


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
                throw ArgTypeException(arg->type(), "bad sql arg type: " + std::to_string((int) arg->type()));
            }            
        }
    }
    return stmt;
}

static int internal::_doUpdate(sql::Connection& connection, SqlBuilder& builder) throw(ArgTypeException)
{
    using Type = SqlArg::Type;
    const std::list<SqlArg*>& args = builder.args();
    std::string sql(builder.sql());
    std::string::size_type pos = sql.find_first_of(internal::SQL_PARAM_HOLDER, 0);
    int k = 1;
    for (std::list<SqlArg*>::const_iterator i = args.cbegin(); i != args.cend(); ++i, k++)
    {
        const SqlArg* arg = *i;
        switch(arg->type())
        {
            case Type::Int:
            {
                std::string s = std::to_string(arg->asInt());
                sql.replace(pos, 1, s);
                pos += s.size() + 1;
                break;
            }
            case Type::Long:
            {
                std::string s = std::to_string(arg->asLong());
                sql.replace(pos, 1, s);
                pos += s.size() + 1;
                break;
            }
            case Type::Bool:
            {
                std::string s = std::to_string(arg->asBool());
                sql.replace(pos, 1, s);
                pos += s.size() + 1;
                break;
            }
            case Type::UInt:
            {
                std::string s = std::to_string(arg->asUInt());
                sql.replace(pos, 1, s);
                pos += s.size() + 1;
                break;
            }
            case Type::ULong:
            {
                std::string s = std::to_string(arg->asULong());
                sql.replace(pos, 1, s);
                pos += s.size() + 1;
                break;
            }
            case Type::Double:
            {
                std::string s = std::to_string(arg->asDouble());
                sql.replace(pos, 1, s);
                pos += s.size() + 1;
                break;
            }
            case Type::String:
            {
                std::string s = std::string(arg->asString()->asStdString());
                s = internal::escape(s, "\\", "\\\\");
                s = internal::escape(s, "'", "\\'");
                s.insert(s.begin(), '\'');
                s.insert(s.end(), '\'');
                sql.replace(pos, 1, s);
                pos += s.size() + 1;
                break;
            }
            case Type::Blob:
            {
                std::string s = std::to_string(arg->asInt());
                sql.replace(pos, 1, s);
                pos += s.size() + 1;
                break;
            }
            default:
            {
                throw ArgTypeException(arg->type(), "bad sql arg type: " + std::to_string((int) arg->type()));
            }            
        }
        pos = sql.find_first_of(internal::SQL_PARAM_HOLDER, pos);
    }
    std::shared_ptr<sql::Statement> stmt(connection.createStatement());
    return stmt->executeUpdate(sql);
}

LC_SQL_DECL_END

#endif
