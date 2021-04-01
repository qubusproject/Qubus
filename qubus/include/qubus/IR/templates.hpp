#ifndef QUBUS_IR_TEMPLATES_HPP
#define QUBUS_IR_TEMPLATES_HPP

#include <qubus/IR/function.hpp>
#include <qubus/IR/type.hpp>
#include <qubus/IR/template_parameter.hpp>
#include <qubus/IR/compile_time_evaluator.hpp>

#include <qubus/util/integers.hpp>

#include <variant>

namespace qubus
{

class type_template
{
public:
    type instantiate(const std::vector<compile_time_value>& args) const;

private:
    std::vector<template_parameter> parameters_;
    type template_;
};

class function_template
{
public:
    function instantiate(const std::vector<compile_time_value>& args) const;

private:
    std::vector<template_parameter> parameters_;
    function template_;
};

} // namespace qubus

#endif
