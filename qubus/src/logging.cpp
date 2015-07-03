#include <qbb/qubus/logging.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <iostream>
#include <fstream>
#include <ostream>
#include <iomanip>
#include <memory>

namespace qbb
{
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

}

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
    typedef sinks::synchronous_sink<sinks::text_ostream_backend> text_sink;
    boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();

    sink->locked_backend()->add_stream(boost::make_shared<std::ofstream>("kubus.log"));

    sink->set_formatter(
        expr::stream << std::hex << std::setw(8) << std::setfill('0') << line_id << std::dec
                     << std::setfill(' ') << ": <" << severity << ">\t"
                     << "(" << scope << ") "
                     << expr::if_(expr::has_attr(tag_attr))[expr::stream << "[" << tag_attr << "] "]
                     << "[" << timestamp_utc << "] "
                     << expr::smessage);

    logging::core::get()->add_sink(sink);

    // Add attributes
    logging::add_common_attributes();
    logging::core::get()->add_global_attribute("Scope", attrs::named_scope());
    logging::core::get()->add_global_attribute("TimeStampUTC", attrs::utc_clock());

    logging::core::get()->set_filter(severity >= normal);
}

}
}