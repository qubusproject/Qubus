#ifndef QBB_UTIL_DLL_HPP
#define QBB_UTIL_DLL_HPP

#include <qbb/util/exception.hpp>

#include <boost/filesystem/path.hpp>

#include <string>

inline namespace qbb
{
namespace util
{

class dll
{
public:
    explicit dll(const std::string& filename);
    explicit dll(const boost::filesystem::path& p);
    ~dll();

    template<typename SymbolType>
    SymbolType* lookup_symbol(const std::string& symbol) const
    {
        return reinterpret_cast<SymbolType*>(lookup_symbol_raw(symbol));
    }
    
    boost::filesystem::path get_directory() const;
private:
    void* lookup_symbol_raw(const std::string& symbol) const;
    
    void* handle_;
};
}
}

#endif