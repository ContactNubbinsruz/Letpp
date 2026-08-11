#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <type_traits>
#include <stdexcept>
#include <utility>
#include <sstream>
#include <cstddef>
#include <initializer_list>

class let
{
public:

    using array = std::vector<let>;
    using object = std::vector<std::pair<std::string, let>>;

    using Value = std::variant<
        std::monostate,
        int,
        double,
        bool,
        std::string,
        array,
        object
    >;

private:

    Value value;

public:

    // constructors

    let()
        : value(std::monostate{})
    {}

    let(std::nullptr_t)
        : value(std::monostate{})
    {}

    let(int v)
        : value(v)
    {}

    let(double v)
        : value(v)
    {}

    let(float v)
        : value(static_cast<double>(v))
    {}

    let(bool v)
        : value(v)
    {}

    let(const char* v)
        : value(std::string(v))
    {}

    let(std::string_view v)
        : value(std::string(v))
    {}

    let(const std::string& v)
        : value(v)
    {}

    let(std::string&& v)
        : value(std::move(v))
    {}

    let(const array& v)
        : value(v)
    {}

    let(array&& v)
        : value(std::move(v))
    {}

    let(const object& v)
        : value(v)
    {}

    let(object&& v)
        : value(std::move(v))
    {}

    let(std::initializer_list<let> v)
        : value(buildFromList(v))
    {}

    // type checks

    static Value buildFromList(std::initializer_list<let> v)
    {
        // "object" pattern: every element is a 2-item array whose
        // first item is a string, e.g. { {"name", "Amelia"}, {"age", 15} }

        object result;

        for (const auto& item : v)
        {
            if (!item.isArray() ||
                item.size() != 2 ||
                !std::get<array>(item.value)[0].isString())
            {
                return array(v);
            }

            const auto& pairArr = std::get<array>(item.value);

            result.emplace_back(
                std::get<std::string>(pairArr[0].value),
                pairArr[1]
            );
        }

        return v.size() > 0 ? Value(result) : Value(array(v));
    }

    bool isNull() const
    {
        return std::holds_alternative<std::monostate>(value);
    }

    bool isInt() const
    {
        return std::holds_alternative<int>(value);
    }

    bool isDouble() const
    {
        return std::holds_alternative<double>(value);
    }

    bool isBool() const
    {
        return std::holds_alternative<bool>(value);
    }

    bool isString() const
    {
        return std::holds_alternative<std::string>(value);
    }

    bool isArray() const
    {
        return std::holds_alternative<array>(value);
    }

    bool isObject() const
    {
        return std::holds_alternative<object>(value);
    }

    bool isNumber() const
    {
        return isInt() || isDouble();
    }

    // string conversion

    std::string str() const
    {
        return stringify(value);
    }

    static std::string stringify(const Value& v)
    {
        return std::visit(
            [](const auto& x) -> std::string
            {
                using T = std::decay_t<decltype(x)>;

                if constexpr (
                    std::is_same_v<T, std::monostate>
                )
                {
                    return "null";
                }

                else if constexpr (
                    std::is_same_v<T, std::string>
                )
                {
                    return x;
                }

                else if constexpr (
                    std::is_same_v<T, bool>
                )
                {
                    return x ? "true" : "false";
                }

                else if constexpr (
                    std::is_same_v<T, array>
                )
                {
                    std::string result = "[";

                    for (std::size_t i = 0; i < x.size(); ++i)
                    {
                        if (i != 0)
                            result += ", ";

                        result += stringify(x[i].value);
                    }

                    result += "]";

                    return result;
                }

                else if constexpr (
                    std::is_same_v<T, object>
                )
                {
                    std::string result = "{";

                    for (std::size_t i = 0; i < x.size(); ++i)
                    {
                        if (i != 0)
                            result += ", ";

                        result += "\"" + x[i].first + "\": ";
                        result += stringify(x[i].second.value);
                    }

                    result += "}";

                    return result;
                }

                else if constexpr (
                    std::is_same_v<T, double>
                )
                {
                    std::ostringstream oss;
                    oss << x;
                    return oss.str();
                }

                else
                {
                    return std::to_string(x);
                }
            },
            v
        );
    }

    // numeric conversion

    static double number(const Value& v)
    {
        return std::visit(
            [](const auto& x) -> double
            {
                using T = std::decay_t<decltype(x)>;

                if constexpr (
                    std::is_same_v<T, int> ||
                    std::is_same_v<T, double> ||
                    std::is_same_v<T, bool>
                )
                {
                    return static_cast<double>(x);
                }

                else
                {
                    throw std::runtime_error(
                        "let: value is not numeric"
                    );
                }
            },
            v
        );
    }

    // +

    friend let operator+(const let& a, const let& b)
    {
        // string concatenation

        if (a.isString() || b.isString())
        {
            return
                stringify(a.value) +
                stringify(b.value);
        }

        // integer + integer

        if (a.isInt() && b.isInt())
        {
            return
                std::get<int>(a.value) +
                std::get<int>(b.value);
        }

        // numeric

        return
            number(a.value) +
            number(b.value);
    }

    // -

    friend let operator-(const let& a, const let& b)
    {
        if (a.isString() || b.isString())
        {
            throw std::runtime_error(
                "let: cannot subtract strings"
            );
        }

        if (a.isInt() && b.isInt())
        {
            return
                std::get<int>(a.value) -
                std::get<int>(b.value);
        }

        return
            number(a.value) -
            number(b.value);
    }

    // *

    friend let operator*(const let& a, const let& b)
    {
        // string * number

        if (a.isString() && b.isNumber())
        {
            const auto& str =
                std::get<std::string>(a.value);

            int count =
                static_cast<int>(
                    number(b.value)
                );

            if (count < 0)
            {
                throw std::runtime_error(
                    "let: negative string repetition"
                );
            }

            std::string result;

            result.reserve(
                str.size() * count
            );

            for (int i = 0; i < count; ++i)
                result += str;

            return result;
        }

        // number * string

        if (a.isNumber() && b.isString())
            return b * a;

        // int * int

        if (a.isInt() && b.isInt())
        {
            return
                std::get<int>(a.value) *
                std::get<int>(b.value);
        }

        return
            number(a.value) *
            number(b.value);
    }

    // /

    friend let operator/(const let& a, const let& b)
    {
        double divisor =
            number(b.value);

        if (divisor == 0.0)
        {
            throw std::runtime_error(
                "let: division by zero"
            );
        }

        if (a.isInt() && b.isInt())
        {
            return
                std::get<int>(a.value) /
                std::get<int>(b.value);
        }

        return
            number(a.value) /
            divisor;
    }

    // %

    friend let operator%(const let& a, const let& b)
    {
        int x =
            static_cast<int>(
                number(a.value)
            );

        int y =
            static_cast<int>(
                number(b.value)
            );

        if (y == 0)
        {
            throw std::runtime_error(
                "let: modulo by zero"
            );
        }

        return x % y;
    }


    friend bool operator==(const let& a, const let& b)
    {
        return std::visit(
            [](const auto& x, const auto& y) -> bool
            {
                using A = std::decay_t<decltype(x)>;
                using B = std::decay_t<decltype(y)>;

                // numeric comparison

                if constexpr (
                    (
                        std::is_same_v<A, int> ||
                        std::is_same_v<A, double>
                    ) &&
                    (
                        std::is_same_v<B, int> ||
                        std::is_same_v<B, double>
                    )
                )
                {
                    return x == y;
                }

                // same types

                else if constexpr (
                    std::is_same_v<A, B>
                )
                {
                    return x == y;
                }

                else
                {
                    return false;
                }
            },
            a.value,
            b.value
        );
    }

    friend bool operator!=(const let& a, const let& b)
    {
        return !(a == b);
    }

    // <

    friend bool operator<(const let& a, const let& b)
    {
        if (a.isString() && b.isString())
        {
            return
                std::get<std::string>(a.value) <
                std::get<std::string>(b.value);
        }

        return
            number(a.value) <
            number(b.value);
    }

    friend bool operator>(const let& a, const let& b)
    {
        return b < a;
    }

    friend bool operator<=(const let& a, const let& b)
    {
        return !(a > b);
    }

    friend bool operator>=(const let& a, const let& b)
    {
        return !(a < b);
    }

    // !

    friend bool operator!(const let& a)
    {
        return std::visit(
            [](const auto& x) -> bool
            {
                using T = std::decay_t<decltype(x)>;

                if constexpr (
                    std::is_same_v<T, std::monostate>
                )
                {
                    return true;
                }

                else if constexpr (
                    std::is_same_v<T, std::string>
                )
                {
                    return x.empty();
                }

                else if constexpr (
                    std::is_same_v<T, array> ||
                    std::is_same_v<T, object>
                )
                {
                    return x.empty();
                }

                else
                {
                    return !static_cast<bool>(x);
                }
            },
            a.value
        );
    }

    explicit operator bool() const
    {
        return !(!*this);
    }

    // array indexing

    let& operator[](int index)
    {
        return (*this)[static_cast<long>(index)];
    }

    const let& operator[](int index) const
    {
        return (*this)[static_cast<long>(index)];
    }

    let& operator[](long index)
    {
        if (!isArray())
        {
            throw std::runtime_error(
                "let: value is not an array"
            );
        }

        auto& arr =
            std::get<array>(value);

        if (index < 0 || static_cast<std::size_t>(index) >= arr.size())
        {
            throw std::out_of_range(
                "let: array index " +
                std::to_string(index) +
                " out of range (size " +
                std::to_string(arr.size()) +
                ")"
            );
        }

        return arr[static_cast<std::size_t>(index)];
    }

    const let& operator[](long index) const
    {
        if (!isArray())
        {
            throw std::runtime_error(
                "let: value is not an array"
            );
        }

        const auto& arr =
            std::get<array>(value);

        if (index < 0 || static_cast<std::size_t>(index) >= arr.size())
        {
            throw std::out_of_range(
                "let: array index " +
                std::to_string(index) +
                " out of range (size " +
                std::to_string(arr.size()) +
                ")"
            );
        }

        return arr[static_cast<std::size_t>(index)];
    }

    // object indexing (by string key)

    let& operator[](const std::string& key)
    {
        if (!isObject())
        {
            throw std::runtime_error(
                "let: value is not an object"
            );
        }

        auto& obj = std::get<object>(value);

        for (auto& kv : obj)
            if (kv.first == key)
                return kv.second;

        // key not present: create it (like std::map)
        obj.push_back(std::make_pair(key, let()));
        return obj.back().second;
    }

    const let& operator[](const std::string& key) const
    {
        if (!isObject())
        {
            throw std::runtime_error(
                "let: value is not an object"
            );
        }

        const auto& obj = std::get<object>(value);

        for (const auto& kv : obj)
            if (kv.first == key)
                return kv.second;

        throw std::out_of_range(
            "let: object has no key \"" + key + "\""
        );
    }

    let& operator[](const char* key)
    {
        return (*this)[std::string(key)];
    }

    const let& operator[](const char* key) const
    {
        return (*this)[std::string(key)];
    }

    let& operator[](const let& key)
    {
        if (isObject())
            return (*this)[std::get<std::string>(key.value)];

        return (*this)[static_cast<long>(number(key.value))];
    }

    const let& operator[](const let& key) const
    {
        if (isObject())
            return (*this)[std::get<std::string>(key.value)];

        return (*this)[static_cast<long>(number(key.value))];
    }

    std::size_t size() const
    {
        if (isArray())
            return std::get<array>(value).size();

        if (isObject())
            return std::get<object>(value).size();

        if (isString())
            return std::get<std::string>(value).size();

        throw std::runtime_error(
            "let: size() called on non-container"
        );
    }

    // key / value (for {key, value} pairs from object iteration)

    let key() const
    {
        if (!isArray() || size() != 2)
        {
            throw std::runtime_error(
                "let: key() only valid on a {key, value} pair"
            );
        }

        return (*this)[0];
    }

    let val() const
    {
        if (!isArray() || size() != 2)
        {
            throw std::runtime_error(
                "let: val() only valid on a {key, value} pair"
            );
        }

        return (*this)[1];
    }

    // empty / clear

    bool empty() const
    {
        return size() == 0;
    }

    void clear()
    {
        if (isArray())
        {
            std::get<array>(value).clear();
            return;
        }

        if (isObject())
        {
            std::get<object>(value).clear();
            return;
        }

        if (isString())
        {
            std::get<std::string>(value).clear();
            return;
        }

        throw std::runtime_error(
            "let: clear() called on non-container"
        );
    }

    // push

    void push(const let& v)
    {
        if (!isArray())
        {
            throw std::runtime_error(
                "let: push() called on non-array"
            );
        }

        std::get<array>(value).push_back(v);
    }

    template<typename T>
    void push(T&& v)
    {
        push(let(std::forward<T>(v)));
    }

    // iteration

    // unified iterator: yields each element by reference for
    // arrays, or a {key, value} let-array (by value) for objects.
    struct iterator
    {
        let* self;
        std::size_t i;

        let& operator*() const
        {
            if (self->isObject())
            {
                auto& kv = std::get<object>(self->value)[i];

                // object entries are exposed as {key, value} pairs;
                // this scratch is read-only in practice.
                static thread_local let scratch;
                scratch = array{ kv.first, kv.second };
                return scratch;
            }

            return std::get<array>(self->value)[i];
        }

        iterator& operator++()
        {
            ++i;
            return *this;
        }

        bool operator!=(const iterator& other) const
        {
            return i != other.i;
        }
    };

    struct const_iterator
    {
        const let* self;
        std::size_t i;

        let operator*() const
        {
            if (self->isObject())
            {
                const auto& kv = std::get<object>(self->value)[i];
                return array{ kv.first, kv.second };
            }

            return std::get<array>(self->value)[i];
        }

        const_iterator& operator++()
        {
            ++i;
            return *this;
        }

        bool operator!=(const const_iterator& other) const
        {
            return i != other.i;
        }
    };

    static std::size_t containerSize(const let& v)
    {
        if (v.isObject())
            return std::get<object>(v.value).size();

        return std::get<array>(v.value).size();
    }

    iterator begin()
    {
        return iterator{ this, 0 };
    }

    iterator end()
    {
        return iterator{ this, containerSize(*this) };
    }

    const_iterator begin() const
    {
        return const_iterator{ this, 0 };
    }

    const_iterator end() const
    {
        return const_iterator{ this, containerSize(*this) };
    }

    // contains / find

    bool contains(const let& v) const
    {
        if (!isArray())
        {
            throw std::runtime_error(
                "let: contains() called on non-array"
            );
        }

        const auto& arr = std::get<array>(value);

        for (const auto& item : arr)
            if (item == v)
                return true;

        return false;
    }

    long find(const let& v) const
    {
        if (!isArray())
        {
            throw std::runtime_error(
                "let: find() called on non-array"
            );
        }

        const auto& arr = std::get<array>(value);

        for (std::size_t i = 0; i < arr.size(); ++i)
            if (arr[i] == v)
                return static_cast<long>(i);

        return -1;
    }

    // type name (used by log)

    const Value& raw() const
    {
        return value;
    }

    static std::string typeName(const Value& v)
    {
        return std::visit(
            [](const auto& x) -> std::string
            {
                using T = std::decay_t<decltype(x)>;

                if constexpr (std::is_same_v<T, std::monostate>) return "null";
                else if constexpr (std::is_same_v<T, int>)       return "int";
                else if constexpr (std::is_same_v<T, double>)    return "double";
                else if constexpr (std::is_same_v<T, bool>)      return "bool";
                else if constexpr (std::is_same_v<T, std::string>) return "string";
                else if constexpr (std::is_same_v<T, array>)     return "array";
                else if constexpr (std::is_same_v<T, object>)    return "object";
            },
            v
        );
    }

    // ostream

    friend std::ostream& operator<<(
        std::ostream& os,
        const let& v
    )
    {
        os << stringify(v.value);
        return os;
    }

    // assignment

    template<typename T>
    let& operator=(T&& v)
    {
        value = let(
            std::forward<T>(v)
        ).value;

        return *this;
    }

    // compound assignment

    let& operator+=(const let& rhs) { return *this = *this + rhs; }
    let& operator-=(const let& rhs) { return *this = *this - rhs; }
    let& operator*=(const let& rhs) { return *this = *this * rhs; }
    let& operator/=(const let& rhs) { return *this = *this / rhs; }
    let& operator%=(const let& rhs) { return *this = *this % rhs; }

    // increment / decrement

    let& operator++()
    {
        return *this = *this + let(1);
    }

    let operator++(int)
    {
        let tmp = *this;
        ++(*this);
        return tmp;
    }

    let& operator--()
    {
        return *this = *this - let(1);
    }

    let operator--(int)
    {
        let tmp = *this;
        --(*this);
        return tmp;
    }
};

// free-standing log

inline void log(const let& v)
{
    std::cout << v << " (" << let::typeName(v.raw()) << ")\n";
}

using array = let::array;
using obj = let::object;


// primitive operators

template<typename T>
let operator+(T lhs, const let& rhs)
{
    return let(lhs) + rhs;
}

template<typename T>
let operator+(const let& lhs, T rhs)
{
    return lhs + let(rhs);
}


template<typename T>
let operator-(T lhs, const let& rhs)
{
    return let(lhs) - rhs;
}

template<typename T>
let operator-(const let& lhs, T rhs)
{
    return lhs - let(rhs);
}


template<typename T>
let operator*(T lhs, const let& rhs)
{
    return let(lhs) * rhs;
}

template<typename T>
let operator*(const let& lhs, T rhs)
{
    return lhs * let(rhs);
}


template<typename T>
let operator/(T lhs, const let& rhs)
{
    return let(lhs) / rhs;
}

template<typename T>
let operator/(const let& lhs, T rhs)
{
    return lhs / let(rhs);
}


template<typename T>
let operator%(T lhs, const let& rhs)
{
    return let(lhs) % rhs;
}

template<typename T>
let operator%(const let& lhs, T rhs)
{
    return lhs % let(rhs);
}


// c-string + let

inline let operator+(const char* lhs, const let& rhs)
{
    return let(lhs) + rhs;
}

inline let operator+(const let& lhs, const char* rhs)
{
    return lhs + let(rhs);
}


// c-string - let

inline let operator-(const char* lhs, const let& rhs)
{
    return let(lhs) - rhs;
}

inline let operator-(const let& lhs, const char* rhs)
{
    return lhs - let(rhs);
}


// c-string * let

inline let operator*(const char* lhs, const let& rhs)
{
    return let(lhs) * rhs;
}

inline let operator*(const let& lhs, const char* rhs)
{
    return lhs * let(rhs);
}


// c-string / let

inline let operator/(const char* lhs, const let& rhs)
{
    return let(lhs) / rhs;
}

inline let operator/(const let& lhs, const char* rhs)
{
    return lhs / let(rhs);
}
