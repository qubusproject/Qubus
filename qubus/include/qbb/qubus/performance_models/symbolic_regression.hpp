#ifndef QUBUS_PERFORMANCE_MODELS_SYMBOLIC_REGRESSION_HPP
#define QUBUS_PERFORMANCE_MODELS_SYMBOLIC_REGRESSION_HPP

#include <boost/optional.hpp>

#include <chrono>
#include <vector>
#include <memory>

namespace qbb
{
namespace qubus
{

class symbolic_regression_impl;

class symbolic_regression
{
public:
    explicit symbolic_regression(long int window_size_);
    ~symbolic_regression();

    symbolic_regression(const symbolic_regression&) = delete;
    symbolic_regression& operator=(const symbolic_regression&) = delete;

    symbolic_regression(symbolic_regression&&);
    symbolic_regression& operator=(symbolic_regression&&);

    void add_datapoint(std::vector<double> arguments, std::chrono::microseconds execution_time);

    boost::optional<std::chrono::microseconds> update();

    boost::optional<std::chrono::microseconds> update_cheaply();

    long int size_of_dataset() const;

    boost::optional<std::chrono::microseconds> query(const std::vector<double>& arguments) const;
    boost::optional<std::chrono::microseconds> accuracy() const;

private:
    std::unique_ptr<symbolic_regression_impl> impl_;
};

}
}

#endif
