#ifndef QUBUS_PATTERN_TYPE_HPP
#define QUBUS_PATTERN_TYPE_HPP

#include <qubus/IR/type.hpp>

#include <qubus/pattern/any.hpp>
#include <qubus/pattern/sequence.hpp>
#include <qubus/pattern/value.hpp>
#include <qubus/pattern/variable.hpp>

#include <utility>

namespace qubus
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
        real_type_.reset();
    }

private:
    RealType real_type_;
};

template <typename TensorType, typename ValueType, typename Rank>
class array_type_pattern
{
public:
    explicit array_type_pattern(ValueType value_type_, Rank rank_)
    : value_type_(std::move(value_type_)), rank_(std::move(rank_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<TensorType>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<TensorType>())
        {
            if (value_type_.match(concret_value->value_type()))
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
        }

        return false;
    }

    void reset() const
    {
        value_type_.reset();
        rank_.reset();
    }

private:
    ValueType value_type_;
    Rank rank_;
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

template <typename Id, typename Members>
class struct_type_pattern
{
public:
    struct_type_pattern(Id id_, Members members_)
    : id_(std::move(id_)), members_(std::move(members_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<types::struct_>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<types::struct_>())
        {
            if (id_.match(concret_value->id()))
            {
                if (members_.match(concret_value->members()))
                {
                    if (var)
                    {
                        var->set(*concret_value);
                    }

                    return true;
                }
            }
        }

        return false;
    }

    void reset() const
    {
        id_.reset();
        members_.reset();
    }

private:
    Id id_;
    Members members_;
};

template <typename Datatype, typename Id>
class member_pattern
{
public:
    member_pattern(Datatype datatype_, Id id_) : datatype_(datatype_), id_(id_)
    {
    }

    bool match(const types::struct_::member& value,
               const variable<types::struct_::member>* var = nullptr) const
    {
        if (datatype_.match(value.datatype))
        {
            if (id_.match(value.id))
            {
                if (var)
                {
                    var->set(value);
                }

                return true;
            }
        }

        return false;
    }

    void reset() const
    {
        datatype_.reset();
        id_.reset();
    }

private:
    Datatype datatype_;
    Id id_;
};

constexpr primitive_type_pattern<types::double_> double_t = {};
constexpr primitive_type_pattern<types::float_> float_t = {};
constexpr primitive_type_pattern<types::integer> integer_t = {};
constexpr primitive_type_pattern<types::bool_> bool_t = {};
constexpr primitive_type_pattern<types::index> index_t = {};

template <typename RealType>
complex_type_pattern<RealType> complex_t(RealType real_type)
{
    return complex_type_pattern<RealType>(std::move(real_type));
}

template <typename Rank>
multi_index_type_pattern<Rank> multi_index_t(Rank rank)
{
    return multi_index_type_pattern<Rank>(std::move(rank));
}

template <typename ValueType, typename Rank>
array_type_pattern<types::array, ValueType, Rank> array_t(ValueType value_type, Rank rank)
{
    return array_type_pattern<types::array, ValueType, Rank>(std::move(value_type),
                                                             std::move(rank));
}

template <typename ValueType, typename Rank>
array_type_pattern<types::array_slice, ValueType, Rank> array_slice_t(ValueType value_type,
                                                                      Rank rank)
{
    return array_type_pattern<types::array_slice, ValueType, Rank>(std::move(value_type),
                                                                   std::move(rank));
}

template <typename Id, typename Members>
struct_type_pattern<Id, Members> struct_t(Id id, Members members)
{
    return struct_type_pattern<Id, Members>(std::move(id), std::move(members));
}

template <typename Datatype, typename Id>
auto member(Datatype datatype, Id id)
{
    return member_pattern<Datatype, Id>(datatype, id);
}

template <typename ValueType>
auto sparse_tensor_t(ValueType value_type)
{
    return struct_t(
        value("sparse_tensor"),
        sequence(member(struct_t(_, sequence(member(array_t(value_type, _), value("val")), _, _, _)),
                        value("data")),
                 member(_, value("shape"))));
}
}
}

#endif