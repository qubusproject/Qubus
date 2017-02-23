#ifndef QUBUS_UTIL_ASSERT_HPP
#define QUBUS_UTIL_ASSERT_HPP

#include <boost/assert.hpp>

#define QUBUS_ASSERT(COND,MSG) BOOST_ASSERT_MSG(COND,MSG)

#define QUBUS_VERIFY(COND,MSG) BOOST_VERIFY_MSG(COND,MSG)

#endif