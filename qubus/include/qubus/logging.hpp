#ifndef QUBUS_LOGGING_HPP
#define QUBUS_LOGGING_HPP

#include <boost/log/common.hpp>
#include <boost/version.hpp>

#include <ostream>

namespace qubus
{

enum severity_level
{
    info,
    normal,
    notification,
    warning,
    error,
    critical
};

std::ostream& operator<<(std::ostream& strm, severity_level level);

using logger = boost::log::sources::severity_logger_mt<severity_level>;

void init_logging();
void finalize_logging();
}

#if BOOST_VERSION >= 105900 && BOOST_VERSION < 106000
// Workaround for Boost.Log bug #11549.

#define QUBUS_LOG(lg, sev) BOOST_LOG_SEV(lg, sev) << ""

#else

#define QUBUS_LOG(lg, sev) BOOST_LOG_SEV(lg, sev)

#endif

#endif