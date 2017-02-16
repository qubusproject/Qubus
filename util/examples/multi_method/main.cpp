#include <qbb/util/multi_method.hpp>

#include <boost/range/adaptor/transformed.hpp>

#include <atomic>
#include <iostream>
#include <map>
#include <memory>
#include <typeindex>

#include <typeinfo> // for bad_cast

class foo1
{
};

class foo2
{
};

class foo3
{
};

class foo4
{
};

class bar1
{
};

class bar2
{
};

class bar3
{
};

class bar4
{
};

void blub1(const foo1&, const foo2&)
{
}

void blub2(const foo1&, const foo1&)
{
}

void blub3(const foo2&, const foo2&)
{
}

void blub4(const foo2&, const foo1&)
{
}

void blub5(const foo1&, const foo3&)
{
}

void blub6(const foo4&, const foo2&)
{
}

class foo
{
public:
    template <typename T>
    foo(T value)
    {
        auto tag = implementation_table_.register_type<T>();

        self_ = std::make_shared<foo_wrapper<T>>(value, tag);
    }

    std::type_index rtti() const
    {
        return self_->rtti();
    }

    template <typename T>
    T as() const
    {
        using value_type = typename std::decay<T>::type;

        if (self_->rtti() == typeid(value_type))
        {
            return static_cast<foo_wrapper<value_type>*>(self_.get())->get();
        }
        else
        {
            throw std::bad_cast();
        }
    }

    qubus::util::index_t type_tag() const
    {
        return self_->tag();
    }

    static const qubus::util::implementation_table& get_implementation_table()
    {
        return implementation_table_;
    }

    static std::size_t number_of_implementations()
    {
        return implementation_table_.number_of_implementations();
    }

private:
    class foo_interface
    {
    public:
        virtual ~foo_interface()
        {
        }

        virtual std::type_index rtti() const = 0;
        virtual qubus::util::index_t tag() const = 0;
    };

    template <typename T>
    class foo_wrapper final : public foo_interface
    {
    public:
        explicit foo_wrapper(T value_, qubus::util::index_t tag_) : value_(value_), tag_(tag_)
        {
        }

        virtual ~foo_wrapper()
        {
        }

        const T& get() const
        {
            return value_;
        }

        std::type_index rtti() const override
        {
            return typeid(T);
        }

        qubus::util::index_t tag() const override
        {
            return tag_;
        }

    private:
        T value_;
        qubus::util::index_t tag_;
    };

    std::shared_ptr<foo_interface> self_;

    static qubus::util::implementation_table implementation_table_;
};

qubus::util::implementation_table foo::implementation_table_ = {};

class bar
{
public:
    template <typename T>
    bar(T value)
    {
        auto tag = implementation_table_.register_type<T>();

        self_ = std::make_shared<bar_wrapper<T>>(value, tag);
    }

    std::type_index rtti() const
    {
        return self_->rtti();
    }

    template <typename T>
    T as() const
    {
        using value_type = typename std::decay<T>::type;

        if (self_->rtti() == typeid(value_type))
        {
            return static_cast<bar_wrapper<value_type>*>(self_.get())->get();
        }
        else
        {
            throw std::bad_cast();
        }
    }

    qubus::util::index_t type_tag() const
    {
        return self_->tag();
    }

    static const qubus::util::implementation_table& get_implementation_table()
    {
        return implementation_table_;
    }

    static std::size_t number_of_implementations()
    {
        return implementation_table_.number_of_implementations();
    }

private:
    class bar_interface
    {
    public:
        virtual ~bar_interface()
        {
        }

        virtual std::type_index rtti() const = 0;
        virtual qubus::util::index_t tag() const = 0;
    };

    template <typename T>
    class bar_wrapper final : public bar_interface
    {
    public:
        explicit bar_wrapper(T value_, qubus::util::index_t tag_) : value_(value_), tag_(tag_)
        {
        }

        virtual ~bar_wrapper()
        {
        }

        const T& get() const
        {
            return value_;
        }

        std::type_index rtti() const override
        {
            return typeid(T);
        }

        qubus::util::index_t tag() const override
        {
            return tag_;
        }

    private:
        T value_;
        qubus::util::index_t tag_;
    };

    std::shared_ptr<bar_interface> self_;

    static qubus::util::implementation_table implementation_table_;
};

qubus::util::implementation_table bar::implementation_table_ = {};

qubus::util::sparse_multi_method<void(const qubus::util::virtual_<foo>&,
                                      const qubus::util::virtual_<foo>&)>
    blub = {};

qubus::util::sparse_multi_method<void(const qubus::util::virtual_<foo>&,
                                      const qubus::util::virtual_<foo>&,
                                      const qubus::util::virtual_<bar>&)>
    blab = {};

qubus::util::multi_method<void(int, const qubus::util::virtual_<foo>&,
                               const qubus::util::virtual_<bar>&)>
    blib = {};

void blab1(const foo1&, const foo2&, const bar1&)
{
}

void blib1(int, const foo2&, const bar1&)
{
}

int main(int, char**)
{
    std::cout << "Hello, world!" << std::endl;

    blub.add_specialization(blub1);
    blub.add_specialization(blub2);
    blub.add_specialization(blub3);
    blub.add_specialization(blub4);
    blub.add_specialization(blub5);
    blub.add_specialization(blub6);

    blab.add_specialization(blab1);

    blib.add_specialization(blib1);

    foo foo1_value = foo1{};
    foo foo2_value = foo2{};
    foo foo3_value = foo3{};
    foo foo4_value = foo4{};

    bar bar1_value = bar1{};
    bar bar2_value = bar2{};
    bar bar3_value = bar3{};
    bar bar4_value = bar4{};

    for (int i = 0; i < 100000000; ++i)
    {
        blub(foo1_value, foo2_value);
        blab(foo1_value, foo2_value, bar1_value);
        blib(0, foo2_value, bar1_value);
    }

    return 0;
}
