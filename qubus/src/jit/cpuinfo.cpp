#include <qubus/jit/cpuinfo.hpp>

#include <llvm/Support/Host.h>

#include <boost/range/iterator_range_core.hpp>

#include <fstream>
#include <regex>
#include <map>

namespace qubus
{

#if defined(__x86_64__) && defined(__linux__)

std::vector<std::string> deduce_host_cpu_features()
{
    std::ifstream cpuinfo("/proc/cpuinfo");

    if (!cpuinfo.is_open())
    {
        throw 0;
    }

    std::string line;

    std::map<std::string, std::string> known_features = {{"mmx", "mmx"},
                                                         {"sse", "sse"},
                                                         {"sse2", "sse2"},
                                                         {"sse3", "sse3"},
                                                         {"ssse3", "ssse3"},
                                                         {"sse4_1", "sse4.1"},
                                                         {"sse4_2", "sse4.2"},
                                                         {"sse4a", "sse4a"},
                                                         {"avx", "avx"},
                                                         {"avx2", "avx2"},
                                                         {"avx512f", "avx512f"},
                                                         {"fma", "fma"},
                                                         {"fma4", "fma4"},
                                                         {"popcnt", "popcnt"},
                                                         {"movbe", "movbe"},
                                                         {"bmi1", "bmi"},
                                                         {"bmi2", "bmi2"}};

    while (std::getline(cpuinfo, line))
    {
        std::regex pattern("^flags");

        std::smatch results;

        if (std::regex_search(line, results, pattern))
        {
            auto feature_list_start = line.begin() + line.find(':') + 1;

            std::regex word_pattern("(\\S+)");

            auto features_begin =
                std::sregex_iterator(feature_list_start, line.end(), word_pattern);
            auto features_end = std::sregex_iterator();

            std::vector<std::string> features;

            for (const auto& match : boost::make_iterator_range(features_begin, features_end))
            {
                auto iter = known_features.find(match.str());

                if (iter != known_features.end())
                {
                    features.push_back(iter->second);
                }
            }

            return features;
        }
    }

    return {};
}

#else

#error "deduce_host_cpu_features is not implemented for this architecture."

#endif

std::vector<std::string> get_host_cpu_features()
{
    std::vector<std::string> available_features;
    llvm::StringMap<bool> features;

    if (llvm::sys::getHostCPUFeatures(features))
    {
        for (const auto& feature : features)
        {
            if (feature.getValue())
            {
                available_features.push_back(feature.getKey());
            }
        }
    }
    else
    {
        available_features = deduce_host_cpu_features();
    }

    return  available_features;
}

#if defined(__x86_64__) && defined(__linux__)

util::index_t get_prefered_alignment()
{
    return 8;
}

#else

#error "get_prefered_alignment is not implemented for this architecture."

#endif

}