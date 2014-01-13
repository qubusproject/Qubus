#ifndef QBB_ASSERT_HPP
#define QBB_ASSERT_HPP

#include <boost/assert.hpp>

#define QBB_ASSERT(COND,MSG) BOOST_ASSERT_MSG(COND,MSG)

#endif