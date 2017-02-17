#include <qubus/jit/operators.hpp>

#include <qubus/jit/compiler.hpp>

#include <qubus/jit/load_store.hpp>
#include <qubus/jit/entry_block_alloca.hpp>

#include <qubus/pattern/core.hpp>
#include <qubus/pattern/IR.hpp>

#include <qubus/IR/type_inference.hpp>

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>

namespace qubus
{
namespace jit
{
reference emit_binary_operator(binary_op_tag tag, const expression& left, const expression& right, compiler& comp)
{
    using pattern::_;

    auto& env = comp.get_module().env();
    auto& ctx = comp.get_module().ctx();

    reference left_value_ptr = comp.compile(left);
    reference right_value_ptr = comp.compile(right);

    auto& builder = env.builder();

    llvm::Instruction* left_value = load_from_ref(left_value_ptr, env, ctx);

    llvm::Instruction* right_value = load_from_ref(right_value_ptr, env, ctx);

    type result_type = typeof_(left);

    llvm::Value* result;

    switch (tag)
    {
        case binary_op_tag::assign:
        {
            store_to_ref(left_value_ptr, right_value, env, ctx);

            return reference();
        }
        case binary_op_tag::plus_assign:
        {
            auto m =
                    pattern::make_matcher<type, llvm::Value*>()
                            .case_(pattern::float_t || pattern::double_t,
                                   [&]
                                   {
                                       return builder.CreateFAdd(left_value, right_value);
                                   })
                            .case_(pattern::integer_t,
                                   [&]
                                   {
                                       return builder.CreateAdd(left_value, right_value);
                                   })
                            .case_(pattern::complex_t(_), [&]
                            {
                                llvm::Value* lhs_real =
                                        builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 0});
                                llvm::Value* lhs_imag =
                                        builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 1});

                                llvm::Value* rhs_real =
                                        builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 0});
                                llvm::Value* rhs_imag =
                                        builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 1});

                                llvm::Value* result = llvm::UndefValue::get(left_value->getType());

                                llvm::Value* result_real = builder.CreateFAdd(lhs_real, rhs_real);
                                llvm::Value* result_imag = builder.CreateFAdd(lhs_imag, rhs_imag);

                                result = builder.CreateInsertValue(result, result_real,
                                                                   std::vector<unsigned>{0, 0});
                                result = builder.CreateInsertValue(result, result_imag,
                                                                   std::vector<unsigned>{0, 1});

                                return result;
                            });

            llvm::Value* sum = pattern::match(result_type, m);

            store_to_ref(left_value_ptr, sum, env, ctx);

            return reference();
        }
        case binary_op_tag::plus:
        {
            auto m =
                    pattern::make_matcher<type, llvm::Value*>()
                            .case_(pattern::float_t || pattern::double_t,
                                   [&]
                                   {
                                       return builder.CreateFAdd(left_value, right_value);
                                   })
                            .case_(pattern::integer_t,
                                   [&]
                                   {
                                       return builder.CreateAdd(left_value, right_value, "", true, true);
                                   })
                            .case_(pattern::complex_t(_), [&]
                            {
                                llvm::Value* lhs_real =
                                        builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 0});
                                llvm::Value* lhs_imag =
                                        builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 1});

                                llvm::Value* rhs_real =
                                        builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 0});
                                llvm::Value* rhs_imag =
                                        builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 1});

                                llvm::Value* result = llvm::UndefValue::get(left_value->getType());

                                llvm::Value* result_real = builder.CreateFAdd(lhs_real, rhs_real);
                                llvm::Value* result_imag = builder.CreateFAdd(lhs_imag, rhs_imag);

                                result = builder.CreateInsertValue(result, result_real,
                                                                   std::vector<unsigned>{0, 0});
                                result = builder.CreateInsertValue(result, result_imag,
                                                                   std::vector<unsigned>{0, 1});

                                return result;
                            });

            result = pattern::match(result_type, m);

            break;
        }
        case binary_op_tag::minus:
        {
            auto m =
                    pattern::make_matcher<type, llvm::Value*>()
                            .case_(pattern::float_t || pattern::double_t,
                                   [&]
                                   {
                                       return builder.CreateFSub(left_value, right_value);
                                   })
                            .case_(pattern::integer_t,
                                   [&]
                                   {
                                       return builder.CreateSub(left_value, right_value, "", true, true);
                                   })
                            .case_(pattern::complex_t(_), [&]
                            {
                                llvm::Value* lhs_real =
                                        builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 0});
                                llvm::Value* lhs_imag =
                                        builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 1});

                                llvm::Value* rhs_real =
                                        builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 0});
                                llvm::Value* rhs_imag =
                                        builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 1});

                                llvm::Value* result = llvm::UndefValue::get(left_value->getType());

                                llvm::Value* result_real = builder.CreateFSub(lhs_real, rhs_real);
                                llvm::Value* result_imag = builder.CreateFSub(lhs_imag, rhs_imag);

                                result = builder.CreateInsertValue(result, result_real,
                                                                   std::vector<unsigned>{0, 0});
                                result = builder.CreateInsertValue(result, result_imag,
                                                                   std::vector<unsigned>{0, 1});

                                return result;
                            });

            result = pattern::match(result_type, m);

            break;
        }
        case binary_op_tag::multiplies:
        {
            auto m =
                    pattern::make_matcher<type, llvm::Value*>()
                            .case_(pattern::float_t || pattern::double_t,
                                   [&]
                                   {
                                       return builder.CreateFMul(left_value, right_value);
                                   })
                            .case_(pattern::integer_t,
                                   [&]
                                   {
                                       return builder.CreateMul(left_value, right_value, "", true, true);
                                   })
                            .case_(pattern::complex_t(_), [&]
                            {
                                llvm::Value* lhs_real =
                                        builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 0});
                                llvm::Value* lhs_imag =
                                        builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 1});

                                llvm::Value* rhs_real =
                                        builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 0});
                                llvm::Value* rhs_imag =
                                        builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 1});

                                llvm::Value* result = llvm::UndefValue::get(left_value->getType());

                                llvm::Value* prod_rr = builder.CreateFMul(lhs_real, rhs_real);
                                llvm::Value* prod_ii = builder.CreateFMul(lhs_imag, rhs_imag);
                                llvm::Value* result_real = builder.CreateFSub(prod_rr, prod_ii);

                                llvm::Value* prod_ri = builder.CreateFMul(lhs_real, rhs_imag);
                                llvm::Value* prod_ir = builder.CreateFMul(lhs_imag, rhs_real);
                                llvm::Value* result_imag = builder.CreateFAdd(prod_ri, prod_ir);

                                result = builder.CreateInsertValue(result, result_real,
                                                                   std::vector<unsigned>{0, 0});
                                result = builder.CreateInsertValue(result, result_imag,
                                                                   std::vector<unsigned>{0, 1});

                                return result;
                            });

            result = pattern::match(result_type, m);

            break;
        }
        case binary_op_tag::divides:
        {
            auto m =
                    pattern::make_matcher<type, llvm::Value*>()
                            .case_(pattern::float_t || pattern::double_t,
                                   [&]
                                   {
                                       return builder.CreateFDiv(left_value, right_value);
                                   })
                            .case_(pattern::integer_t,
                                   [&]
                                   {
                                       return builder.CreateSDiv(left_value, right_value);
                                   })
                            .case_(pattern::complex_t(_), [&]
                            {
                                llvm::Value* lhs_real =
                                        builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 0});
                                llvm::Value* lhs_imag =
                                        builder.CreateExtractValue(left_value, std::vector<unsigned>{0, 1});

                                llvm::Value* rhs_real =
                                        builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 0});
                                llvm::Value* rhs_imag =
                                        builder.CreateExtractValue(right_value, std::vector<unsigned>{0, 1});

                                llvm::Value* result = llvm::UndefValue::get(left_value->getType());

                                llvm::Value* rhs_norm =
                                        builder.CreateFAdd(builder.CreateFMul(rhs_real, rhs_real),
                                                           builder.CreateFMul(rhs_imag, rhs_imag));

                                llvm::Value* prod_rr = builder.CreateFMul(lhs_real, rhs_real);
                                llvm::Value* prod_ii = builder.CreateFMul(lhs_imag, rhs_imag);
                                llvm::Value* result_real = builder.CreateFAdd(prod_rr, prod_ii);

                                llvm::Value* prod_ri = builder.CreateFMul(lhs_real, rhs_imag);
                                llvm::Value* prod_ir = builder.CreateFMul(lhs_imag, rhs_real);
                                llvm::Value* result_imag = builder.CreateFSub(prod_ir, prod_ri);

                                result_real = builder.CreateFDiv(result_real, rhs_norm);
                                result_imag = builder.CreateFDiv(result_imag, rhs_norm);

                                result = builder.CreateInsertValue(result, result_real,
                                                                   std::vector<unsigned>{0, 0});
                                result = builder.CreateInsertValue(result, result_imag,
                                                                   std::vector<unsigned>{0, 1});

                                return result;
                            });

            result = pattern::match(result_type, m);

            break;
        }
        case binary_op_tag::modulus:
        {
            auto m = pattern::make_matcher<type, llvm::Value*>()
                    .case_(pattern::float_t || pattern::double_t,
                           [&]
                           {
                               return builder.CreateFRem(left_value, right_value);
                           })
                    .case_(pattern::integer_t, [&]
                    {
                        return builder.CreateSRem(left_value, right_value);
                    });

            result = pattern::match(result_type, m);

            break;
        }
        case binary_op_tag::div_floor:
        {
            auto m = pattern::make_matcher<type, llvm::Value*>().case_(
                    pattern::integer_t, [&]
            {
                llvm::Type* integer_type = env.map_qubus_type(types::integer{});

                auto quotient = builder.CreateSDiv(left_value, right_value);
                auto remainder = builder.CreateSRem(left_value, right_value);

                auto is_exact =
                        builder.CreateICmpEQ(remainder, llvm::ConstantInt::get(integer_type, 0, true));
                auto is_positive = builder.CreateICmpEQ(
                        builder.CreateICmpSLT(remainder, llvm::ConstantInt::get(integer_type, 0, true)),
                        builder.CreateICmpSLT(quotient, llvm::ConstantInt::get(integer_type, 0, true)));
                auto is_exact_or_positive = builder.CreateOr(is_exact, is_positive);

                auto reduced_quotient =
                        builder.CreateSub(quotient, llvm::ConstantInt::get(integer_type, 1, true));

                return builder.CreateSelect(is_exact_or_positive, quotient, reduced_quotient);
            });

            result = pattern::match(result_type, m);

            break;
        }
        case binary_op_tag::equal_to:
        {
            auto m = pattern::make_matcher<type, llvm::Value*>().case_(
                    pattern::integer_t || pattern::bool_t, [&]
            {
                return builder.CreateICmpEQ(left_value, right_value);
            });

            result = pattern::match(result_type, m);

            break;
        }
        case binary_op_tag::less:
        {
            auto m = pattern::make_matcher<type, llvm::Value*>().case_(
                    pattern::integer_t || pattern::bool_t, [&]
            {
                return builder.CreateICmpSLT(left_value, right_value);
            });

            result = pattern::match(result_type, m);

            break;
        }
        case binary_op_tag::less_equal:
        {
            auto m = pattern::make_matcher<type, llvm::Value*>().case_(
                    pattern::integer_t || pattern::bool_t, [&]
            {
                return builder.CreateICmpSLE(left_value, right_value);
            });

            result = pattern::match(result_type, m);

            break;
        }
        case binary_op_tag::greater:
        {
            auto m = pattern::make_matcher<type, llvm::Value*>().case_(
                    pattern::integer_t || pattern::bool_t, [&]
            {
                return builder.CreateICmpSGT(left_value, right_value);
            });

            result = pattern::match(result_type, m);

            break;
        }
        case binary_op_tag::greater_equal:
        {
            auto m = pattern::make_matcher<type, llvm::Value*>().case_(
                    pattern::integer_t || pattern::bool_t, [&]
            {
                return builder.CreateICmpSGE(left_value, right_value);
            });

            result = pattern::match(result_type, m);

            break;
        }
        case binary_op_tag::logical_and:
        {
            auto m = pattern::make_matcher<type, llvm::Value*>().case_(pattern::bool_t, [&]
            {
                return builder.CreateAnd(
                        left_value, right_value);
            });

            result = pattern::match(result_type, m);

            break;
        }
        case binary_op_tag::logical_or:
        {
            auto m = pattern::make_matcher<type, llvm::Value*>().case_(pattern::bool_t, [&]
            {
                return builder.CreateOr(
                        left_value, right_value);
            });

            result = pattern::match(result_type, m);

            break;
        }
        default:
            throw 0;
    }

    auto result_var = create_entry_block_alloca(env.get_current_function(), result->getType());

    reference result_var_ref(result_var, access_path(), result_type);
    store_to_ref(result_var_ref, result, env, ctx);

    return result_var_ref;
}

reference emit_unary_operator(unary_op_tag tag, const expression& arg, compiler& comp)
{
    using pattern::_;

    auto& env = comp.get_module().env();
    auto& ctx = comp.get_module().ctx();

    reference arg_value_ptr = comp.compile(arg);

    auto& builder = env.builder();

    llvm::Instruction* arg_value = load_from_ref(arg_value_ptr, env, ctx);

    type result_type = typeof_(arg);

    llvm::Value* result;

    switch (tag)
    {
        case unary_op_tag::negate:
        {
            auto m =
                    pattern::make_matcher<type, llvm::Value*>()
                            .case_(pattern::float_t || pattern::double_t,
                                   [&]
                                   {
                                       return builder.CreateFNeg(arg_value);
                                   })
                            .case_(pattern::integer_t,
                                   [&]
                                   {
                                       return builder.CreateNeg(arg_value);
                                   })
                            .case_(pattern::complex_t(_), [&]
                            {
                                llvm::Value* real =
                                        builder.CreateExtractValue(arg_value, std::vector<unsigned>{0, 0});
                                llvm::Value* imag =
                                        builder.CreateExtractValue(arg_value, std::vector<unsigned>{0, 1});

                                llvm::Value* result = llvm::UndefValue::get(arg_value->getType());

                                llvm::Value* result_real = builder.CreateFNeg(real);
                                llvm::Value* result_imag = builder.CreateFNeg(imag);

                                result = builder.CreateInsertValue(result, result_real,
                                                                   std::vector<unsigned>{0, 0});
                                result = builder.CreateInsertValue(result, result_imag,
                                                                   std::vector<unsigned>{0, 1});

                                return result;
                            });

            result = pattern::match(result_type, m);

            break;
        }
        case unary_op_tag::plus:
            result = arg_value;
            break;
        default:
            throw 0;
    }

    auto result_var = create_entry_block_alloca(env.get_current_function(), result->getType());

    reference result_var_ref(result_var, access_path(), result_type);

    store_to_ref(result_var_ref, result, env, ctx);

    return result_var_ref;
}
}
}