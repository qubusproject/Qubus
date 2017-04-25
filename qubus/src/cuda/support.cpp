#include <qubus/cuda/support.hpp>

#include <hpx/runtime/threads/thread_helpers.hpp>
#include <hpx/runtime_fwd.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/unused.hpp>

#include <boost/intrusive_ptr.hpp>

namespace qubus
{
namespace cuda
{

namespace
{

struct runtime_registration_wrapper
{
    runtime_registration_wrapper(hpx::runtime* rt) : rt_(rt)
    {
        QUBUS_ASSERT(rt, "Object was not properly initialized.");

        hpx::error_code ec(hpx::lightweight); // ignore errors
        hpx::register_thread(rt_, "qubus-cuda", ec);
    }

    ~runtime_registration_wrapper()
    {
        hpx::unregister_thread(rt_);
    }

    hpx::runtime* rt_;
};

///////////////////////////////////////////////////////////////////////
struct future_data : hpx::lcos::detail::future_data<void>
{
private:
    static void stream_callback(CUstream stream, CUresult status, void* user_data);

public:
    future_data();

    void init(stream& stream);

private:
    hpx::runtime* rt_;
};

struct release_on_exit
{
    release_on_exit(future_data* data) : data_(data)
    {
    }

    ~release_on_exit()
    {
        // release the shared state
        hpx::lcos::detail::intrusive_ptr_release(data_);
    }

    future_data* data_;
};

void future_data::stream_callback(CUstream QUBUS_UNUSED(stream), CUresult status, void* user_data)
{
    future_data* this_ = static_cast<future_data*>(user_data);

    runtime_registration_wrapper wrap(this_->rt_);

    // We need to run this as an HPX thread ...
    hpx::applier::register_thread_nullary(
        [this_, status]() {
            release_on_exit on_exit(this_);

            if (status != CUDA_SUCCESS)
            {
                this_->set_exception(boost::copy_exception(cuda_error(status)));
            }

            this_->set_data(hpx::util::unused);
        },
        "qubus::cuda::future_data_stream_callback");
}

future_data::future_data() : rt_(hpx::get_runtime_ptr())
{
}

void future_data::init(stream& s)
{
    try
    {
        // Hold on to the shared state on behalf of the cuda runtime
        // right away as the callback could be called immediately.
        hpx::lcos::detail::intrusive_ptr_add_ref(this);

        auto error = cuStreamAddCallback(s.native_handle(), stream_callback, this, 0);

        check_cuda_error(error);
    }
    catch (...)
    {
        hpx::lcos::detail::intrusive_ptr_release(this);

        throw;
    }
}
}

hpx::future<void> when_finished(stream& s)
{
    using shared_state_type = future_data;

    // Make sure shared state stays alive even if the callback is invoked
    // during initialization.
    boost::intrusive_ptr<shared_state_type> p(new shared_state_type());

    p->init(s);

    return hpx::traits::future_access<hpx::future<void>>::create(std::move(p));
}
}
}