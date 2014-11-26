#ifndef QBB_KUBUS_LOGGING_HPP
#define QBB_KUBUS_LOGGING_HPP

#include <boost/log/common.hpp>

#include <ostream>

namespace qbb
{
namespace kubus
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

using logger = boost::log::sources::severity_logger<severity_level>;

void init_logging();
    
}
}

#endif