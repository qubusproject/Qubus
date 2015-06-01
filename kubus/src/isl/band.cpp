#include <qbb/kubus/isl/band.hpp>

namespace qbb
{
namespace kubus
{
namespace isl
{

band::band(isl_band* handle_) : handle_(handle_)
{
}

band::band(const band& other) : handle_(isl_band_copy(other.native_handle()))
{
}

band::~band()
{
    isl_band_free(handle_);
}

context_ref band::get_ctx() const
{
    return context_ref(isl_band_get_ctx(handle_));
}

bool band::has_children() const
{
    return isl_band_has_children(handle_);
}

namespace
{
extern "C" isl_stat add_band_child(isl_band* child, void* user) noexcept;
}

std::vector<band> band::get_children() const
{
    std::vector<band> result;

    isl_band_list* children = isl_band_get_children(handle_);

    isl_band_list_foreach(children, add_band_child, &result);

    isl_band_list_free(children);

    return result;
}

union_map band::get_partial_schedule() const
{
    return union_map(isl_band_get_partial_schedule(handle_));
}

union_map band::get_prefix_schedule() const
{
    return union_map(isl_band_get_prefix_schedule(handle_));
}

union_map band::get_suffix_schedule() const
{
    return union_map(isl_band_get_suffix_schedule(handle_));
}

int band::n_member() const
{
    return isl_band_n_member(handle_);
}

isl_band* band::native_handle() const
{
    return handle_;
}

isl_band* band::release() noexcept
{
    isl_band* temp = handle_;

    handle_ = nullptr;

    return temp;
}

void tile_band(band& b, const std::vector<qbb::util::index_t>& sizes)
{
    int n = qbb::util::integer_cast<int>(sizes.size());

    isl_vec* sizes_ = isl_vec_alloc(b.get_ctx().native_handle(), n);

    for (int i = 0; i < n; ++i)
    {
        isl_vec_set_element_si(sizes_, i, qbb::util::integer_cast<int>(sizes[i]));
    }

    isl_band_tile(b.native_handle(), sizes_);
}

namespace
{
extern "C" {

isl_stat add_band_child(isl_band* child, void* user) noexcept
{
    auto& children = *static_cast<std::vector<band>*>(user);

    children.emplace_back(child);

    return isl_stat_ok;
}
}
}
}
}
}