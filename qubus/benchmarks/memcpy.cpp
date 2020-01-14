#include <nonius/nonius.h++>

#include <vector>
#include <cstring>

void memcpy(const std::vector<char>& input, std::vector<char>& output)
{
    std::memcpy(output.data(), input.data(), output.size());
}

int main()
{
    nonius::configuration cfg;
    cfg.output_file = "memcpy.html";

    std::vector<char> input(100*1024*1024);
    std::vector<char> target(100*1024*1024);

    nonius::benchmark benchmarks[] = {
        nonius::benchmark("100 MB memcpy", [&]{ memcpy(input, target); })};

    nonius::go(cfg, std::begin(benchmarks), std::end(benchmarks), nonius::html_reporter());

    nonius::configuration cfg2;

    nonius::go(cfg2, std::begin(benchmarks), std::end(benchmarks), nonius::standard_reporter());
}