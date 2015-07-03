#ifndef QBB_QUBUS_ISL_ID_HPP
#define QBB_QUBUS_ISL_ID_HPP

#include <isl/id.h>

#include <boost/any.hpp>

#include <string>

namespace qbb
{
namespace qubus
{
namespace isl
{

class context;

class id
{
public:
    explicit id(isl_id* handle_);
    id(const context& ctx, const std::string& name);
    id(const context& ctx, const std::string& name, boost::any user_data);

    id(const id& other);

    ~id();

    isl_id* native_handle() const;

    isl_id* release() noexcept;

    std::string name() const;

    const boost::any& user_data() const;
    boost::any& user_data();

private:
    isl_id* handle_;
};
}
}
}

#endif