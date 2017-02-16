#ifndef QBB_UTIL_BOX_HPP
#define QBB_UTIL_BOX_HPP

#include <qbb/util/make_unique.hpp>

#include <memory>
#include <utility>

inline namespace qbb
{
namespace util
{

template <typename T>
class box
{
public:
    box() = default;
    explicit box(T content_) : content_(util::make_unique<T>(std::move(content_)))
    {
    }

    box(const box&) = delete;
    box(box&&) = default;

    box& operator=(const box&) = delete;
    box& operator=(box&&) = default;

    explicit operator bool() const
    {
        return static_cast<bool>(content_);
    }

    const T& value() const
    {
        return *content_;
    }

    T& value()
    {
        return *content_;
    }

    const T& operator*() const
    {
        return value();
    }

    T& operator*()
    {
        return value();
    }

    const T* operator->() const
    {
        return &value();
    }

    T* operator->()
    {
        return &value();
    }

    void fill(T content_)
    {
        // TODO: Assert that the box is empty.

        this->content_ = util::make_unique<T>(std::move(content_));
    }

    void replace(T content_)
    {
        this->content_ = util::make_unique<T>(std::move(content_));
    }

    T empty()
    {
        T content = std::move(*content_);

        content_.reset();

        return content;
    }

private:
    std::unique_ptr<T> content_;
};
}
}

#endif
