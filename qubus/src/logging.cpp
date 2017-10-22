#include <hpx/config.hpp>

#include <qubus/logging.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/lcos/broadcast.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <qubus/util/unused.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <vector>

namespace qubus
{
class hpx_log_server : public hpx::components::component_base<hpx_log_server>
{
public:
    hpx_log_server() : log_file_("qubus.log")
    {
    }

    void consume(const std::string& msg)
    {
        std::lock_guard<hpx::lcos::local::mutex> guard(log_mutex_);

        log_file_ << msg << '\n';
    }

    void flush()
    {
        std::lock_guard<hpx::lcos::local::mutex> guard(log_mutex_);

        log_file_.flush();
    }

    HPX_DEFINE_COMPONENT_ACTION(hpx_log_server, consume, consume_action);
    HPX_DEFINE_COMPONENT_ACTION(hpx_log_server, flush, flush_action);

private:
    mutable hpx::lcos::local::mutex log_mutex_;
    std::ofstream log_file_;
};
}

using server_type = hpx::components::component<qubus::hpx_log_server>;
HPX_REGISTER_COMPONENT(server_type, qubus_hpx_log_server);

using consume_action = qubus::hpx_log_server::consume_action;
HPX_REGISTER_ACTION(consume_action, qubus_hpx_log_server_consume_action);

using flush_action = qubus::hpx_log_server::flush_action;
HPX_REGISTER_ACTION(flush_action, qubus_hpx_log_server_flush_action);

namespace qubus
{
namespace
{
namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(tag_attr, "Tag", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(scope, "Scope", attrs::named_scope::value_type)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp_utc, "TimeStampUTC", attrs::utc_clock::value_type)
BOOST_LOG_ATTRIBUTE_KEYWORD(locality, "Locality", std::uint32_t)

class hpx_log_client : public hpx::components::client_base<hpx_log_client, hpx_log_server>
{
public:
    using base_type = hpx::components::client_base<hpx_log_client, hpx_log_server>;

    hpx_log_client() = default;

    hpx_log_client(hpx::id_type&& id) : base_type(std::move(id))
    {
    }

    hpx_log_client(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
    {
    }

    void consume(const std::string& msg)
    {
        hpx::async<hpx_log_server::consume_action>(this->get_id(), msg).get();
    }

    void flush()
    {
        hpx::async<hpx_log_server::flush_action>(this->get_id()).get();
    }
};

class hpx_log_backend
: public boost::log::sinks::basic_formatted_sink_backend<
      char, boost::log::sinks::combine_requirements<boost::log::sinks::concurrent_feeding,
                                                    boost::log::sinks::flushing>::type>
{
public:
    using base_type = boost::log::sinks::basic_formatted_sink_backend<
        char, boost::log::sinks::combine_requirements<boost::log::sinks::concurrent_feeding,
                                                      boost::log::sinks::flushing>::type>;

    using char_type = typename base_type::char_type;
    using string_type = typename base_type::string_type;

    void add_client(hpx_log_client client)
    {
        clients_.push_back(std::move(client));
    }

    void consume(const boost::log::record_view& QUBUS_UNUSED(rec), const string_type& msg)
    {
        for (auto& client : clients_)
        {
            client.consume(msg);
        }
    }

    void flush()
    {
        for (auto& client : clients_)
        {
            client.flush();
        }
    }

private:
    std::vector<hpx_log_client> clients_;
};

boost::shared_ptr<sinks::synchronous_sink<hpx_log_backend>> logging_sink;

void init_local_logging(hpx_log_client log_client)
{
    logging_sink = boost::make_shared<sinks::synchronous_sink<hpx_log_backend>>();

    logging_sink->locked_backend()->add_client(std::move(log_client));

    logging_sink->set_formatter(
        expr::stream << std::hex << std::setw(8) << std::setfill('0') << line_id << std::dec
                     << std::setfill(' ') << ": <" << severity << ">\t"
                     << "(" << scope << ") "
                     << expr::if_(expr::has_attr(tag_attr))[expr::stream << "[" << tag_attr << "] "]
                     << '#' << std::setw(8) << std::setfill('0') << locality << " "
                     << "[" << timestamp_utc << "] " << expr::smessage);

    logging::core::get()->add_sink(logging_sink);

    // Add attributes
    logging::add_common_attributes();
    logging::core::get()->add_global_attribute("Scope", attrs::named_scope());
    logging::core::get()->add_global_attribute("TimeStampUTC", attrs::utc_clock());
    logging::core::get()->add_global_attribute(
        "Locality", attrs::make_function([] { return hpx::get_locality_id(); }));

    logging::core::get()->set_filter(severity >= normal);
}
}
}

HPX_PLAIN_ACTION(qubus::init_local_logging, qubus_init_local_logging_action);

namespace qubus
{
std::ostream& operator<<(std::ostream& strm, severity_level level)
{
    static const char* strings[] = {"info",    "normal", "notification",
                                    "warning", "error",  "critical"};

    if (static_cast<std::size_t>(level) < sizeof(strings) / sizeof(*strings))
        strm << strings[level];
    else
        strm << static_cast<int>(level);

    return strm;
}

void init_logging()
{
    auto log_client = hpx::new_<hpx_log_client>(hpx::find_root_locality());

    hpx::lcos::broadcast<qubus_init_local_logging_action>(hpx::find_all_localities(), log_client).get();
}

void finalize_logging()
{
    logging::core::get()->remove_sink(logging_sink);

    logging_sink.reset();
}
}