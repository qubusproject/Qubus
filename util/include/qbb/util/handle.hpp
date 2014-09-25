#ifndef QBB_UTIL_HANDLE_HPP
#define QBB_UTIL_HANDLE_HPP

#include <memory>
#include <ostream>

namespace qbb
{
namespace util
{ 
    
class handle
{
public:
    friend bool operator==(const handle& lhs, const handle& rhs);

    friend bool operator<(const handle& lhs, const handle& rhs);

    friend std::ostream& operator<<(std::ostream& os, const handle& value);

private:
    friend class handle_factory;

    handle();

    std::shared_ptr<char> id_;
};

class handle_factory
{
public:
    handle create() const;
};

}
}

#endif