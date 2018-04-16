#ifndef VALUE_H_NDB
#define VALUE_H_NDB

#include <ndb/error.hpp>

#include <string>
#include <vector>
#include <variant>

namespace ndb
{
    class value
    {
    public:
        template<class T>
		value(T&& v) : value_{ std::forward<T>(v) }
        {}

        template<class T>
        const auto& get() const
        {
            try
            {
                return std::get<T>(value_);
            }
            catch (const std::exception& e)
            {
                ndb_error("Can't get unknown type");
            }
        }

    private:
        std::variant<int, double, float, std::string, std::vector<char>> value_;
    };
} // ndb

#endif // VALUE_H_NDB
