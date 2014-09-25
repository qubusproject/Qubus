#ifndef QBB_KUBUS_ISL_SPACE_HPP
#define QBB_KUBUS_ISL_SPACE_HPP

#include <isl/space.h>

#include <string>

namespace qbb
{
namespace kubus
{
namespace isl
{

class context;
    
class space
{
public:
    explicit space(isl_space* handle_);

    space(const context& ctx, unsigned int nparam, unsigned int n);

    space(const context& ctx, unsigned int nparam, unsigned int n_in, unsigned int n_out);

    space(const space& other);

    ~space();

    isl_space* native_handle() const;

    isl_space* release() noexcept;

    unsigned dim(isl_dim_type type) const;
    
    void set_tuple_name(isl_dim_type type, const std::string& name);

    void set_dim_name(isl_dim_type type, int pos, const std::string& name);

private:
    isl_space* handle_;
};

space drop_all_dims(space s, isl_dim_type type);

}
}
}

#endif