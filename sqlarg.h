#ifndef LCMYSQL_ARG_H
#define LCMYSQL_ARG_H

#include <cstdint>
#include <cstring>
#include <istream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>

#include "sqldecls.h"

LC_SQL_DECL_BEGIN

class SqlArg
{
private:
    enum SqlType
    {
        Bool,
        Int,
        UInt,
        Long,
        ULong,
        Float,
        Double,
        BigInt,
        String,
        Blob
    };

    union
    {
        bool _bool;
        std::int32_t _int;
        std::uint32_t _uint;
        std::int64_t _long;
        std::uint64_t _ulong;
        double _double;
        std::istream* _blob;
        sql::SQLString* _string;
    };

    SqlType _type;
    std::shared_ptr<std::string> _rawString;
    bool _destroy = true;

public:
    typedef SqlType Type;

    SqlArg(const SqlArg& arg)
    {
        switch(arg._type)
        {
            case Type::Bool:
                _bool = arg._bool;
                break;
            case Type::Int:
                _int = arg._int;
                break;
            case Type::UInt:
                _uint = arg._uint;
                break;
            case Type::Long:
                _long = arg._long;
                break;
            case Type::ULong:
                _ulong = arg._ulong;
                break;
            case Type::String:
                _string = new sql::SQLString(*arg._string);
                _rawString = arg._rawString;
                break;
            case Type::Double:
                _double = arg._double;
                break;
            default:
                break;
        }
        _type = arg._type;
    }
    SqlArg(bool value): _bool(value), _type(Type::Bool) {}
    SqlArg(std::int32_t value): _int(value), _type(Type::Int) {}
    SqlArg(std::uint32_t value): _uint(value), _type(Type::UInt) {}
    SqlArg(std::int64_t value): _long(value), _type(Type::Long) {}
    SqlArg(std::uint64_t value): _ulong(value), _type(Type::ULong) {}
    SqlArg(const char* value): _type(Type::String)
    {
        _rawString.reset(new std::string(value));
        _string = new sql::SQLString(* _rawString);
    }
    SqlArg(const std::string& value): SqlArg(value.c_str()) { _type = Type::String; }
    SqlArg(double value): _double(value), _type(Type::Double) {}
    SqlArg(Type type, const std::string& value): SqlArg(value) { _type = type; }
    
    ~SqlArg()
    {
        if (_type == Type::Blob && _destroy && _blob != 0)
        {
            std::fstream* f = dynamic_cast<std::fstream *>(_blob);
            if (f != 0)
            {
                f->close();
            }
            delete _blob;
        }
        if (_type == Type::String && _destroy && _string != 0)
        {
            delete _string;
        }
    }
    SqlType type() const
    {
        return this->_type;
    }
    bool asBool() const
    {
        return _bool;
    }
    std::int32_t asInt() const
    {
        return _int;
    }
    std::uint32_t asUInt() const
    {
        return _uint;
    }
    std::int64_t asLong() const
    {
        return _long;
    }
    std::uint64_t asULong() const
    {
        return _ulong;
    }
    double asDouble() const
    {
        return _double;
    }
    sql::SQLString* asString() const
    {
        return _string;
    }
    std::istream* asBlob() const
    {
        return _blob;
    }
    const std::string toString() const
    {
        std::stringstream toStringBuilder;
        toStringBuilder << "SqlArg(";
        switch(_type)
        {
            case Type::Bool:
            {
                toStringBuilder << "Bool";
                toStringBuilder << ": " << _bool << ")";
                break;
            }
            case Type::Int:
            {
                toStringBuilder << "Int";
                toStringBuilder << ": " << _int << ")";
                break;
            }
            case Type::UInt:
            {
                toStringBuilder << "UInt";
                toStringBuilder << ": " << _uint << ")";
                break;
            }
            case Type::Long:
            {
                toStringBuilder << "Long";
                toStringBuilder << ": " << _long << ")";
                break;
            }
            case Type::ULong:
            {
                toStringBuilder << "ULong";
                toStringBuilder << ": " << _ulong << ")";
                break;
            }
            case Type::String:
            {
                toStringBuilder << "String";
                toStringBuilder << ": " << * _rawString << ")";
                break;
            }
            case Type::Double:
            {
                toStringBuilder << "Double";
                toStringBuilder << ": " << _double << ")";
                break;
            }
            default:
                break;
        }
        return toStringBuilder.str();
    }
};

LC_SQL_DECL_END

#endif
