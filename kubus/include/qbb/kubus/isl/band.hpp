#ifndef QBB_KUBUS_ISL_BAND_HPP
#define QBB_KUBUS_ISL_BAND_HPP

#include <qbb/kubus/isl/context.hpp>
#include <qbb/kubus/isl/map.hpp>

#include <isl/band.h>

#include <qbb/util/integers.hpp>

#include <vector>

namespace qbb
{
namespace kubus
{
namespace isl
{

class band
{
public:
    explicit band(isl_band* handle_);

    band(const band& other);

    ~band();
   
    context_ref get_ctx() const;
    
    bool has_children() const;
    std::vector<band> get_children() const;
    
    union_map get_partial_schedule() const;
    union_map get_prefix_schedule() const;
    union_map get_suffix_schedule() const;
    int n_member() const; 
    
    isl_band* native_handle() const;

    isl_band* release() noexcept;
private:
    isl_band* handle_;
};

void tile_band(band& b, const std::vector<qbb::util::index_t>& sizes);

}
}
}

#endif