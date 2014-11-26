#ifndef QBB_UTIL_ASSERT_HPP
#define QBB_UTIL_ASSERT_HPP

#include <boost/assert.hpp>

#define QBB_ASSERT(COND,MSG) BOOST_ASSERT_MSG(COND,MSG)

#define QBB_VERIFY(COND,MSG) BOOST_VERIFY_MSG(COND,MSG)

#endif