#ifndef QBB_KUBUS_PATTERN_TYPE_HPP
#define QBB_KUBUS_PATTERN_TYPE_HPP

#include <qbb/kubus/IR/type.hpp>

#include <qbb/kubus/pattern/variable.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
namespace pattern
{

template <typename T>
class primitive_type_pattern
{
public:
    template <typename BaseType>
    bool match(const BaseType& value, const variable<T>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<T>())
        {
            if (var)
            {
                var->set(*concret_value);
            }

            return true;
        }

        return false;
    }

    void reset() const
    {
    }
};

template <typename RealType>
class complex_type_pattern
{
public:
    complex_type_pattern(RealType real_type_) : real_type_(std::move(real_type_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<types::complex>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<types::complex>())
        {
            if (real_type_.match(concret_value->real_type()))
            {
                if (var)
                {
                    var->set(*concret_value);
                }

                return true;
            }
        }

        return false;
    }

    void reset() const
    {
    }

private:
    RealType real_type_;
};

template <typename TensorType, typename ValueType>
class tensor_type_pattern
{
public:
    tensor_type_pattern(ValueType value_type_) : value_type_(std::move(value_type_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<TensorType>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<TensorType>())
        {
            if (value_type_.match(concret_value->value_type()))
            {
                if (var)
                {
                    var->set(*concret_value);
                }

                return true;
            }
        }

        return false;
    }

    void reset() const
    {
    }

private:
    ValueType value_type_;
};

template <typename Rank>
class multi_index_type_pattern
{
public:
    multi_index_type_pattern(Rank rank_) : rank_(std::move(rank_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<types::multi_index>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<types::multi_index>())
        {
            if (rank_.match(concret_value->rank()))
            {
                if (var)
                {
                    var->set(*concret_value);
                }

                return true;
            }
        }

        return false;
    }

    void reset() const
    {
        rank_.reset();
    }

private:
    Rank rank_;
};

constexpr primitive_type_pattern<types::double_> double_t = {};
constexpr primitive_type_pattern<types::float_> float_t = {};
constexpr primitive_type_pattern<types::integer> integer_t = {};
constexpr primitive_type_pattern<types::index> index_t = {};

template <typename RealType>
complex_type_pattern<RealType> complex_t(RealType real_type)
{
    return complex_type_pattern<RealType>(std::move(real_type));
}

template <typename ValueType>
tensor_type_pattern<types::tensor, ValueType> tensor_t(ValueType value_type)
{
    return tensor_type_pattern<types::tensor, ValueType>(std::move(value_type));
}

template <typename ValueType>
tensor_type_pattern<types::sparse_tensor, ValueType> sparse_tensor_t(ValueType value_type)
{
    return tensor_type_pattern<types::sparse_tensor, ValueType>(std::move(value_type));
}

template <typename Rank>
multi_index_type_pattern<Rank> multi_index_t(Rank rank)
{
    return multi_index_type_pattern<Rank>(std::move(rank));
}

}
}
}

#endif