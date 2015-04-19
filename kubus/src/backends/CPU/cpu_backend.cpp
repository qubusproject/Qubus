#include <hpx/config.hpp>

#include <qbb/kubus/backends/cpu_backend.hpp>

#include <qbb/kubus/backend.hpp>

#include <qbb/kubus/backends/llvm_environment.hpp>
#include <qbb/kubus/backends/cpuinfo.hpp>

#include <qbb/kubus/backends/cpu_allocator.hpp>
#include <qbb/kubus/backends/cpu_memory_block.hpp>
#include <qbb/kubus/local_address_space.hpp>

#include <qbb/kubus/backends/cpu_object_factory.hpp>

#include <qbb/kubus/abi_info.hpp>

#include <qbb/kubus/backends/execution_stack.hpp>
#include <qbb/kubus/metadata_builder.hpp>

#include <qbb/kubus/IR/kir.hpp>
#include <qbb/kubus/pattern/core.hpp>
#include <qbb/kubus/pattern/IR.hpp>

#include <qbb/kubus/IR/type_inference.hpp>

#include <qbb/util/make_unique.hpp>

#include <hpx/async.hpp>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/JITEventListener.h>
#include <llvm/Support/TargetSelect.h>

#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#include <llvm/PassManager.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/DataLayout.h>

#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/Vectorize.h>

#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/Host.h>

#include <boost/optional.hpp>

#include <qbb/util/make_unique.hpp>
#include <qbb/util/assert.hpp>

#include <iostream>
#include <memory>
#include <map>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <algorithm>

namespace qbb
{
namespace kubus
{

namespace
{

class reference
{
public:
    reference() = default;

    reference(llvm::Value* addr_, access_path origin_) : addr_(addr_), origin_(origin_)
    {
    }

    llvm::Value* addr() const
    {
        return addr_;
    }

    const access_path& origin() const
    {
        return origin_;
    }

private:
    llvm::Value* addr_;
    access_path origin_;
};

llvm::AllocaInst* create_entry_block_alloca(llvm::Function* current_function, llvm::Type* type,
                                            llvm::Value* array_size = 0,
                                            const llvm::Twine& name = "")
{
    llvm::IRBuilder<> builder(&current_function->getEntryBlock(),
                              current_function->getEntryBlock().begin());
    return builder.CreateAlloca(type, array_size, name);
}

class compilation_context
{
public:
    std::map<qbb::util::handle, reference>& symbol_table()
    {
        return symbol_table_;
    }
    
    const std::map<qbb::util::handle, reference>& symbol_table() const
    {
        return symbol_table_;
    }
    
    boost::optional<function_declaration> get_next_plan_to_compile()
    {
        if (plans_to_compile_.empty())
            return boost::none;
        
        auto next_plan_to_compile = plans_to_compile_.back();
        
        plans_to_compile_.pop_back();
        
        return next_plan_to_compile;
    }
    
    void add_plan_to_compile(function_declaration fn)
    {
        plans_to_compile_.push_back(std::move(fn));
    }
private:
    std::map<qbb::util::handle, reference> symbol_table_;
    std::vector<function_declaration> plans_to_compile_; 
};

reference compile(const expression& expr, llvm_environment& env,
                  compilation_context& ctx);

template <typename BodyEmitter>
void emit_loop(reference induction_variable, llvm::Value* lower_bound, llvm::Value* upper_bound,
               llvm::Value* increment, BodyEmitter body_emitter, llvm_environment& env)
{
    auto& builder_ = env.builder();

    auto initialize_induction_variable =
        builder_.CreateStore(lower_bound, induction_variable.addr());
    initialize_induction_variable->setMetadata("tbaa",
                                               env.get_tbaa_node(induction_variable.origin()));

    llvm::BasicBlock* header = llvm::BasicBlock::Create(llvm::getGlobalContext(), "header",
                                                        builder_.GetInsertBlock()->getParent());
    llvm::BasicBlock* body = llvm::BasicBlock::Create(llvm::getGlobalContext(), "body",
                                                      builder_.GetInsertBlock()->getParent());
    llvm::BasicBlock* exit = llvm::BasicBlock::Create(llvm::getGlobalContext(), "exit",
                                                      builder_.GetInsertBlock()->getParent());

    builder_.CreateBr(header);

    builder_.SetInsertPoint(header);

    auto induction_variable_value = builder_.CreateLoad(induction_variable.addr());
    induction_variable_value->setMetadata("tbaa", env.get_tbaa_node(induction_variable.origin()));

    llvm::Value* exit_cond = builder_.CreateICmpSLT(induction_variable_value, upper_bound);

    builder_.CreateCondBr(exit_cond, body, exit);

    builder_.SetInsertPoint(body);

    body_emitter();

    auto induction_variable_value2 = builder_.CreateLoad(induction_variable.addr());
    induction_variable_value2->setMetadata("tbaa", env.get_tbaa_node(induction_variable.origin()));

    auto instr = builder_.CreateStore(
        builder_.CreateAdd(induction_variable_value2, increment, "", false, true),
        induction_variable.addr());
    instr->setMetadata("tbaa", env.get_tbaa_node(induction_variable.origin()));

    builder_.CreateBr(header);

    body = builder_.GetInsertBlock();

    exit->moveAfter(body);

    builder_.SetInsertPoint(exit);
}

reference emit_binary_operator(binary_op_tag tag, const expression& left, const expression& right,
                               llvm_environment& env,
                               compilation_context& ctx)
{
    using pattern::_;

    reference left_value_ptr = compile(left, env, ctx);
    reference right_value_ptr = compile(right, env, ctx);

    auto& builder = env.builder();

    llvm::Instruction* left_value =
        builder.CreateAlignedLoad(left_value_ptr.addr(), get_prefered_alignment());
    left_value->setMetadata("tbaa", env.get_tbaa_node(left_value_ptr.origin()));

    llvm::Instruction* right_value =
        builder.CreateAlignedLoad(right_value_ptr.addr(), get_prefered_alignment());
    right_value->setMetadata("tbaa", env.get_tbaa_node(right_value_ptr.origin()));

    type result_type = typeof_(left);

    llvm::Value* result;

    switch (tag)
    {
    case binary_op_tag::assign:
    {
        llvm::Instruction* store = builder.CreateAlignedStore(right_value, left_value_ptr.addr(),
                                                              get_prefered_alignment());
        store->setMetadata("tbaa", env.get_tbaa_node(left_value_ptr.origin()));

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

        llvm::Instruction* store =
            builder.CreateAlignedStore(sum, left_value_ptr.addr(), get_prefered_alignment());
        store->setMetadata("tbaa", env.get_tbaa_node(left_value_ptr.origin()));

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
                           return builder.CreateSub(left_value, right_value);
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
                           return builder.CreateMul(left_value, right_value);
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
                llvm::Type* integer_type = env.map_kubus_type(types::integer{});

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
    default:
        throw 0;
    }

    auto result_var = create_entry_block_alloca(env.get_current_function(), result->getType());
    result_var->setAlignment(get_prefered_alignment());

    builder.CreateAlignedStore(result, result_var, get_prefered_alignment());

    return reference(result_var, access_path());
}

reference emit_unary_operator(unary_op_tag tag, const expression& arg, llvm_environment& env,
                              compilation_context& ctx)
{
    using pattern::_;

    reference arg_value_ptr = compile(arg, env, ctx);

    auto& builder = env.builder();

    llvm::Instruction* arg_value =
        builder.CreateAlignedLoad(arg_value_ptr.addr(), get_prefered_alignment());
    arg_value->setMetadata("tbaa", env.get_tbaa_node(arg_value_ptr.origin()));

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
    result_var->setAlignment(get_prefered_alignment());

    builder.CreateAlignedStore(result, result_var, get_prefered_alignment());

    return reference(result_var, access_path());
}

reference emit_type_conversion(const type& target_type, const expression& arg,
                               llvm_environment& env,
                               compilation_context& ctx)
{
    using pattern::_;

    reference arg_value_ptr = compile(arg, env, ctx);

    auto& builder = env.builder();

    llvm::Instruction* arg_value =
        builder.CreateAlignedLoad(arg_value_ptr.addr(), get_prefered_alignment());
    arg_value->setMetadata("tbaa", env.get_tbaa_node(arg_value_ptr.origin()));

    type arg_type = typeof_(arg);

    auto m =
        pattern::make_matcher<type, llvm::Value*>()
            .case_(pattern::double_t,
                   [&]
                   {
                       auto m2 = pattern::make_matcher<type, llvm::Value*>()
                                     .case_(complex_t(pattern::double_t),
                                            [&]
                                            {
                                                llvm::Type* complex_type = env.map_kubus_type(
                                                    types::complex(types::double_()));

                                                llvm::Value* result =
                                                    llvm::ConstantAggregateZero::get(complex_type);

                                                result = builder.CreateInsertValue(
                                                    result, arg_value, std::vector<unsigned>{0, 0});

                                                return result;
                                            })
                                     .case_(pattern::double_t, [&]
                                            {
                                                return arg_value;
                                            });

                       return pattern::match(target_type, m2);
                   })
            .case_(pattern::float_t,
                   [&]
                   {
                       auto m2 =
                           pattern::make_matcher<type, llvm::Value*>()
                               .case_(complex_t(pattern::double_t),
                                      [&]
                                      {
                                          llvm::Type* complex_type =
                                              env.map_kubus_type(types::complex(types::double_()));

                                          llvm::Value* result =
                                              llvm::ConstantAggregateZero::get(complex_type);

                                          result = builder.CreateInsertValue(
                                              result,
                                              builder.CreateFPExt(
                                                  arg_value, env.map_kubus_type(types::double_())),
                                              std::vector<unsigned>{0, 0});

                                          return result;
                                      })
                               .case_(complex_t(pattern::float_t),
                                      [&]
                                      {
                                          llvm::Type* complex_type =
                                              env.map_kubus_type(types::complex(types::float_()));

                                          llvm::Value* result =
                                              llvm::ConstantAggregateZero::get(complex_type);

                                          result = builder.CreateInsertValue(
                                              result, arg_value, std::vector<unsigned>{0, 0});

                                          return result;
                                      })
                               .case_(pattern::double_t,
                                      [&]
                                      {
                                          return builder.CreateFPExt(
                                              arg_value, env.map_kubus_type(target_type));
                                      })
                               .case_(pattern::float_t, [&]
                                      {
                                          return arg_value;
                                      });

                       return pattern::match(target_type, m2);
                   })
            .case_(pattern::integer_t,
                   [&]
                   {
                       pattern::variable<type> subtype;

                       auto m2 = pattern::make_matcher<type, llvm::Value*>()
                                     .case_(complex_t(subtype),
                                            [&]
                                            {
                                                llvm::Type* complex_type =
                                                    env.map_kubus_type(target_type);

                                                llvm::Value* result =
                                                    llvm::ConstantAggregateZero::get(complex_type);

                                                result = builder.CreateInsertValue(
                                                    result, builder.CreateSIToFP(
                                                                arg_value,
                                                                env.map_kubus_type(subtype.get())),
                                                    std::vector<unsigned>{0, 0});

                                                return result;
                                            })
                                     .case_(bind_to(pattern::float_t || pattern::double_t, subtype),
                                            [&]
                                            {
                                                return builder.CreateSIToFP(
                                                    arg_value, env.map_kubus_type(subtype.get()));
                                            })
                                     .case_(pattern::integer_t, [&]
                                            {
                                                return arg_value;
                                            });

                       return pattern::match(target_type, m2);
                   })
            .case_(complex_t(pattern::float_t),
                   [&]
                   {
                       auto m2 =
                           pattern::make_matcher<type, llvm::Value*>()
                               .case_(complex_t(pattern::double_t),
                                      [&]
                                      {
                                          llvm::Type* complex_type =
                                              env.map_kubus_type(target_type);

                                          llvm::Value* result = llvm::UndefValue::get(complex_type);

                                          llvm::Value* arg_value_real = builder.CreateExtractValue(
                                              arg_value, std::vector<unsigned>{0, 0});
                                          llvm::Value* arg_value_imag = builder.CreateExtractValue(
                                              arg_value, std::vector<unsigned>{0, 1});

                                          result = builder.CreateInsertValue(
                                              result, builder.CreateFPExt(
                                                          arg_value_real,
                                                          env.map_kubus_type(types::double_())),
                                              std::vector<unsigned>{0, 0});

                                          result = builder.CreateInsertValue(
                                              result, builder.CreateFPExt(
                                                          arg_value_imag,
                                                          env.map_kubus_type(types::double_())),
                                              std::vector<unsigned>{0, 1});

                                          return result;
                                      })
                               .case_(complex_t(pattern::float_t), [&]
                                      {
                                          return arg_value;
                                      });

                       return pattern::match(target_type, m2);
                   })
            .case_(complex_t(pattern::double_t), [&]
                   {
                       auto m2 = pattern::make_matcher<type, llvm::Value*>().case_(
                           complex_t(pattern::double_t), [&]
                           {
                               return arg_value;
                           });

                       return pattern::match(target_type, m2);
                   });

    llvm::Value* result = pattern::match(arg_type, m);

    auto result_var = create_entry_block_alloca(env.get_current_function(), result->getType());
    result_var->setAlignment(get_prefered_alignment());

    builder.CreateAlignedStore(result, result_var, get_prefered_alignment());

    return reference(result_var, access_path());
}

llvm::Value* emit_array_access(llvm::Value* data, const std::vector<llvm::Value*>& shape,
                               const std::vector<llvm::Value*>& indices, llvm_environment& env)
{
    auto& builder = env.builder();

    llvm::Type* size_type = env.map_kubus_type(types::integer());

    llvm::Value* linearized_index = llvm::ConstantInt::get(size_type, 0);

    // for (std::size_t i = indices.size(); i-- > 0;)
    for (std::size_t i = 0; i < indices.size(); i++)
    {
        auto extent = shape[i];

        linearized_index =
            builder.CreateAdd(builder.CreateMul(extent, linearized_index), indices[i]);
    }

    return builder.CreateInBoundsGEP(data, linearized_index);
}

reference emit_array_access(const reference& data, const reference& shape,
                            const std::vector<llvm::Value*>& indices, llvm_environment& env)
{
    auto& builder = env.builder();

    std::vector<llvm::Value*> shape_;
    shape_.reserve(indices.size());

    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        auto extent_ptr = builder.CreateConstInBoundsGEP1_32(shape.addr(), i);

        auto extent = builder.CreateLoad(extent_ptr);
        extent->setMetadata("tbaa", env.get_tbaa_node(shape.origin()));

        shape_.push_back(extent);
    }

    auto accessed_element = emit_array_access(data.addr(), shape_, indices, env);

    return reference(accessed_element, data.origin());
}

reference emit_tensor_access(const reference& tensor, const std::vector<llvm::Value*>& indices,
                             llvm_environment& env)
{
    auto& builder = env.builder();

    llvm::Value* data_ptr = builder.CreateConstInBoundsGEP2_32(tensor.addr(), 0, 0, "data_ptr");
    llvm::Value* shape_ptr = builder.CreateConstInBoundsGEP2_32(tensor.addr(), 0, 1, "shape_ptr");

    auto shape = builder.CreateLoad(shape_ptr, "shape");
    shape->setMetadata("tbaa", env.get_tbaa_node(tensor.origin()));

    auto data = builder.CreateLoad(data_ptr, "data");
    data->setMetadata("tbaa", env.get_tbaa_node(tensor.origin()));

    auto accessed_element =
        emit_array_access(reference(data, tensor.origin() / "data"),
                          reference(shape, tensor.origin() / "shape"), indices, env);

    return accessed_element;
}

reference emit_array_slice_access(const reference& slice, const std::vector<llvm::Value*>& indices,
                                  llvm_environment& env)
{
    auto& builder = env.builder();

    llvm::Value* data_ptr = builder.CreateConstInBoundsGEP2_32(slice.addr(), 0, 0, "data_ptr");
    llvm::Value* shape_ptr = builder.CreateConstInBoundsGEP2_32(slice.addr(), 0, 1, "shape_ptr");
    llvm::Value* origin_ptr = builder.CreateConstInBoundsGEP2_32(slice.addr(), 0, 2, "origin_ptr");

    auto shape = builder.CreateLoad(shape_ptr, "shape");
    shape->setMetadata("tbaa", env.get_tbaa_node(slice.origin()));

    auto data = builder.CreateLoad(data_ptr, "data");
    data->setMetadata("tbaa", env.get_tbaa_node(slice.origin()));

    auto origin = builder.CreateLoad(origin_ptr, "origin");
    origin->setMetadata("tbaa", env.get_tbaa_node(slice.origin()));

    std::vector<llvm::Value*> transformed_indices;
    transformed_indices.reserve(indices.size());

    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        auto origin_component_ptr = builder.CreateConstInBoundsGEP1_32(origin, i);

        auto origin_component = builder.CreateLoad(origin_component_ptr);
        origin_component->setMetadata("tbaa", env.get_tbaa_node(slice.origin() / "origin"));

        auto transformed_index = builder.CreateAdd(origin_component, indices[i], "", true, true);

        transformed_indices.push_back(transformed_index);
    }

    auto accessed_element =
        emit_array_access(reference(data, slice.origin() / "data"),
                          reference(shape, slice.origin() / "shape"), transformed_indices, env);

    return accessed_element;
}

std::vector<llvm::Value*> permute_indices(const std::vector<llvm::Value*>& indices,
                                          const std::vector<util::index_t>& permutation)
{
    std::vector<llvm::Value*> permuted_indices;
    permuted_indices.reserve(indices.size());

    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        permuted_indices.push_back(indices[permutation[i]]);
    }

    return permuted_indices;
}

reference compile(const expression& expr, llvm_environment& env,
                  compilation_context& ctx)
{
    auto& symbol_table = ctx.symbol_table();
    
    pattern::variable<binary_op_tag> btag;
    pattern::variable<unary_op_tag> utag;
    pattern::variable<expression> a, b, c, d;
    pattern::variable<std::vector<expression>> expressions;

    pattern::variable<variable_declaration> idx;
    pattern::variable<variable_declaration> var;
    pattern::variable<function_declaration> plan;

    pattern::variable<util::index_t> ival;
    pattern::variable<float> fval;
    pattern::variable<double> dval;

    pattern::variable<std::string> name;

    pattern::variable<type> t;

    auto& builder = env.builder();

    auto m =
        pattern::make_matcher<expression, reference>()
            /*.case_(
                 binary_operator(pattern::value(binary_op_tag::plus),
                                 binary_operator(pattern::value(binary_op_tag::multiplies), a, b),
                                 c),
                 [&]
                 {
                     reference a_value_ptr = compile(a.get(), env, symbol_table);
                     reference b_value_ptr = compile(b.get(), env, symbol_table);
                     reference c_value_ptr = compile(c.get(), env, symbol_table);

                     auto& builder = env.builder();

                     llvm::Instruction* a_value = builder.CreateAlignedLoad(a_value_ptr.addr(), 32);
                     a_value->setMetadata("tbaa", env.get_tbaa_node(a_value_ptr.origin()));

                     llvm::Instruction* b_value = builder.CreateAlignedLoad(b_value_ptr.addr(), 32);
                     b_value->setMetadata("tbaa", env.get_tbaa_node(b_value_ptr.origin()));

                     llvm::Instruction* c_value = builder.CreateAlignedLoad(c_value_ptr.addr(), 32);
                     c_value->setMetadata("tbaa", env.get_tbaa_node(c_value_ptr.origin()));

                     auto double_type = env.map_kubus_type(types::double_());

                     std::vector<llvm::Type*> arg_types(3, double_type);

                     auto fma_type = llvm::FunctionType::get(double_type, arg_types, false);

                     auto fma = env.module().getOrInsertFunction("llvm.fmuladd.f64", fma_type);

                     llvm::Value* result = builder.CreateCall3(fma, a_value, b_value, c_value);

                     auto result_var = create_entry_block_alloca(env.get_current_function(),
               result->getType());
                     result_var->setAlignment(32);

                     builder.CreateAlignedStore(result, result_var, 32);

                     return reference(result_var, access_path());
                 })*/
            .case_(binary_operator(btag, a, b),
                   [&]
                   {
                       return emit_binary_operator(btag.get(), a.get(), b.get(), env, ctx);
                   })
            .case_(unary_operator(utag, a),
                   [&]
                   {
                       return emit_unary_operator(utag.get(), a.get(), env, ctx);
                   })
            .case_(
                 for_(idx, a, b, c, d),
                 [&]
                 {
                     llvm::Type* size_type = env.map_kubus_type(types::integer());

                     auto increment_ptr = compile(c.get(), env, ctx);
                     auto increment_value = builder.CreateLoad(increment_ptr.addr());
                     increment_value->setMetadata("tbaa",
                                                  env.get_tbaa_node(increment_ptr.origin()));

                     llvm::Value* induction_var = create_entry_block_alloca(
                         env.get_current_function(), size_type, nullptr, "ind");

                     auto induction_var_ref = reference(induction_var, access_path());

                     symbol_table[idx.get().id()] = induction_var_ref;

                     reference lower_bound_ptr = compile(a.get(), env, ctx);
                     reference upper_bound_ptr = compile(b.get(), env, ctx);

                     auto lower_bound = builder.CreateLoad(lower_bound_ptr.addr());
                     lower_bound->setMetadata("tbaa", env.get_tbaa_node(lower_bound_ptr.origin()));
                     auto upper_bound = builder.CreateLoad(upper_bound_ptr.addr());
                     upper_bound->setMetadata("tbaa", env.get_tbaa_node(upper_bound_ptr.origin()));

                     emit_loop(induction_var_ref, lower_bound, upper_bound, increment_value,
                               [&]()
                               {
                                   compile(d.get(), env, ctx);
                               },
                               env);

                     return reference();
                 })
            .case_(compound(expressions),
                   [&]
                   {
                       for (const auto& subexpr : expressions.get())
                       {
                           compile(subexpr, env, ctx);
                       }

                       return reference();
                   })
            .case_(double_literal(dval),
                   [&]
                   {
                       llvm::Type* double_type = env.map_kubus_type(types::double_());

                       llvm::Value* value = llvm::ConstantFP::get(double_type, dval.get());

                       auto& builder = env.builder();

                       auto var =
                           create_entry_block_alloca(env.get_current_function(), double_type);
                       var->setAlignment(get_prefered_alignment());

                       auto lit_store =
                           builder.CreateAlignedStore(value, var, get_prefered_alignment());

                       lit_store->setMetadata("tbaa", env.get_tbaa_node(access_path()));

                       return reference(var, access_path());
                   })
            .case_(float_literal(fval),
                   [&]
                   {
                       llvm::Type* float_type = env.map_kubus_type(types::float_());

                       llvm::Value* value = llvm::ConstantFP::get(float_type, fval.get());

                       auto& builder = env.builder();

                       llvm::Value* var =
                           create_entry_block_alloca(env.get_current_function(), float_type);

                       auto lit_store = builder.CreateStore(value, var);
                       lit_store->setMetadata("tbaa", env.get_tbaa_node(access_path()));

                       return reference(var, access_path());
                   })
            .case_(integer_literal(ival),
                   [&]
                   {
                       llvm::Type* size_type = env.map_kubus_type(types::integer());

                       llvm::Value* value = llvm::ConstantInt::get(size_type, ival.get());

                       auto& builder = env.builder();

                       llvm::Value* var =
                           create_entry_block_alloca(env.get_current_function(), size_type);

                       auto lit_store = builder.CreateStore(value, var);
                       lit_store->setMetadata("tbaa", env.get_tbaa_node(access_path()));

                       return reference(var, access_path());
                   })
            .case_(intrinsic_function_n(pattern::value("delta"), a, b),
                   [&]
                   {
                       // TODO: Test if a and b are integers.

                       auto int_type = env.map_kubus_type(types::integer());

                       auto a_ref = compile(a.get(), env, ctx);
                       auto b_ref = compile(b.get(), env, ctx);

                       auto a_value = builder.CreateLoad(a_ref.addr());
                       a_value->setMetadata("tbaa", env.get_tbaa_node(a_ref.origin()));

                       auto b_value = builder.CreateLoad(b_ref.addr());
                       b_value->setMetadata("tbaa", env.get_tbaa_node(b_ref.origin()));

                       auto cond = builder.CreateICmpEQ(a_value, b_value);

                       auto one = llvm::ConstantInt::get(int_type, 1);
                       auto zero = llvm::ConstantInt::get(int_type, 0);

                       auto result_value = builder.CreateSelect(cond, one, zero);

                       auto result =
                           create_entry_block_alloca(env.get_current_function(), int_type);

                       auto result_store = builder.CreateStore(result_value, result);
                       result_store->setMetadata("tbaa", env.get_tbaa_node(access_path()));

                       return reference(result, access_path());
                   })
            .case_(intrinsic_function_n(pattern::value("extent"), variable_ref(idx), b),
                   [&]
                   {
                       auto tensor = symbol_table[idx.get().id()];

                       llvm::Value* shape_ptr =
                           builder.CreateConstInBoundsGEP2_32(tensor.addr(), 0, 1, "shape_ptr");

                       auto shape = builder.CreateLoad(shape_ptr, "shape");
                       shape->setMetadata("tbaa", env.get_tbaa_node(tensor.origin()));

                       reference dim_ref = compile(b.get(), env, ctx);

                       auto dim = builder.CreateLoad(dim_ref.addr());
                       dim->setMetadata("tbaa", env.get_tbaa_node(dim_ref.origin()));

                       llvm::Value* extent_ptr =
                           builder.CreateInBoundsGEP(shape, dim, "extent_ptr");

                       return reference(extent_ptr, tensor.origin() / "shape");
                   })
            .case_(
                 intrinsic_function_n(pattern::value("min"), a, b),
                 [&]
                 {
                     reference left_value_ptr = compile(a.get(), env, ctx);
                     reference right_value_ptr = compile(b.get(), env, ctx);

                     llvm::Instruction* left_value =
                         builder.CreateAlignedLoad(left_value_ptr.addr(), get_prefered_alignment());
                     left_value->setMetadata("tbaa", env.get_tbaa_node(left_value_ptr.origin()));

                     llvm::Instruction* right_value = builder.CreateAlignedLoad(
                         right_value_ptr.addr(), get_prefered_alignment());
                     right_value->setMetadata("tbaa", env.get_tbaa_node(right_value_ptr.origin()));

                     type result_type = typeof_(a.get());

                     auto cond = builder.CreateICmpSLT(left_value, right_value);

                     auto result = builder.CreateSelect(cond, left_value, right_value);

                     auto result_var =
                         create_entry_block_alloca(env.get_current_function(), result->getType());
                     result_var->setAlignment(get_prefered_alignment());

                     builder.CreateAlignedStore(result, result_var, get_prefered_alignment());

                     return reference(result_var, access_path());
                 })
            .case_(
                 intrinsic_function_n(pattern::value("max"), a, b),
                 [&]
                 {
                     reference left_value_ptr = compile(a.get(), env, ctx);
                     reference right_value_ptr = compile(b.get(), env, ctx);

                     llvm::Instruction* left_value =
                         builder.CreateAlignedLoad(left_value_ptr.addr(), get_prefered_alignment());
                     left_value->setMetadata("tbaa", env.get_tbaa_node(left_value_ptr.origin()));

                     llvm::Instruction* right_value = builder.CreateAlignedLoad(
                         right_value_ptr.addr(), get_prefered_alignment());
                     right_value->setMetadata("tbaa", env.get_tbaa_node(right_value_ptr.origin()));

                     type result_type = typeof_(a.get());

                     auto cond = builder.CreateICmpSLT(left_value, right_value);

                     auto result = builder.CreateSelect(cond, right_value, left_value);

                     auto result_var =
                         create_entry_block_alloca(env.get_current_function(), result->getType());
                     result_var->setAlignment(get_prefered_alignment());

                     builder.CreateAlignedStore(result, result_var, get_prefered_alignment());

                     return reference(result_var, access_path());
                 })
            .case_(type_conversion(t, a),
                   [&]
                   {
                       return emit_type_conversion(t.get(), a.get(), env, ctx);
                   })
            .case_(variable_ref(idx),
                   [&]
                   {
                       return symbol_table[idx.get().id()];
                   })
            .case_(subscription(variable_ref(idx), expressions),
                   [&]
                   {
                       auto ref = symbol_table[idx.get().id()];

                       const auto& indices = expressions.get();

                       std::vector<llvm::Value*> indices_;
                       indices_.reserve(indices.size());

                       for (const auto& index : indices)
                       {
                           auto index_ref = compile(index, env, ctx);

                           auto index_ = builder.CreateLoad(index_ref.addr());
                           index_->setMetadata("tbaa", env.get_tbaa_node(index_ref.origin()));

                           indices_.push_back(index_);
                       }

                       using pattern::_;

                       auto m = pattern::make_matcher<type, reference>()
                                    .case_(pattern::tensor_t(_),
                                           [&]
                                           {
                                               return emit_tensor_access(
                                                   ref, indices_, env);
                                           })
                                    .case_(pattern::array_slice_t(_), [&]
                                           {
                                               return emit_array_slice_access(
                                                   ref, indices_, env);
                                           });

                       return pattern::match(idx.get().var_type(), m);
                   })
            .case_(local_variable_def(var, a),
                   [&]
                   {
                       auto var_type = env.map_kubus_type(var.get().var_type());

                       auto var_ptr =
                           create_entry_block_alloca(env.get_current_function(), var_type);

                       auto init_value_ref = compile(a.get(), env, ctx);

                       auto init_value = builder.CreateLoad(init_value_ref.addr());
                       init_value->setMetadata("tbaa", env.get_tbaa_node(init_value_ref.origin()));

                       auto var_store = builder.CreateStore(var_ptr, init_value);
                       var_store->setMetadata("tbaa", env.get_tbaa_node(access_path()));

                       symbol_table[var.get().id()] = reference(var_ptr, access_path());

                       return reference();
                   })
            .case_(spawn(plan, expressions), [&]
                   {
                       std::vector<llvm::Type*> param_types;

                       for (const auto& param : plan.get().params())
                       {
                           param_types.push_back(env.map_kubus_type(param.var_type()));
                       }

                       llvm::FunctionType* fn_type = llvm::FunctionType::get(
                           llvm::Type::getVoidTy(llvm::getGlobalContext()), param_types, false);

                       auto plan_ptr = env.module().getOrInsertFunction(plan.get().name(), fn_type);
                       
                       ctx.add_plan_to_compile(plan.get());

                       std::vector<llvm::Value*> arguments;

                       for (const auto& arg : expressions.get())
                       {
                           auto arg_ref = compile(arg, env, ctx);
                           auto arg_value = builder.CreateLoad(arg_ref.addr());
                           arg_value->setMetadata("tbaa", env.get_tbaa_node(arg_ref.origin()));

                           arguments.push_back(arg_value);
                       }

                       builder.CreateCall(plan_ptr, arguments);

                       return reference();
                   });

    return pattern::match(expr, m);
}

void compile(const function_declaration& plan, llvm_environment& env,
             compilation_context& ctx)
{
    auto& symbol_table = ctx.symbol_table();
    
    // Prolog
    std::vector<llvm::Type*> param_types;

    for (const auto& param : plan.params())
    {
        param_types.push_back(env.map_kubus_type(param.var_type())->getPointerTo());
    }
    
    param_types.push_back(env.map_kubus_type(plan.result().var_type())->getPointerTo());

    llvm::FunctionType* FT = llvm::FunctionType::get(
        llvm::Type::getVoidTy(llvm::getGlobalContext()), param_types, false);
    
    llvm::Function* compiled_plan =
        llvm::Function::Create(FT, llvm::Function::PrivateLinkage, plan.name(), &env.module());
        
    for (std::size_t i = 0; i < compiled_plan->arg_size(); ++i)
    {
        compiled_plan->setDoesNotAlias(i + 1);
    }
        
    env.set_current_function(compiled_plan);
    
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", compiled_plan);
    env.builder().SetInsertPoint(BB);
    
    // body

    auto current_arg = compiled_plan->arg_begin();
    
    for (const auto& param : plan.params())
    {
        symbol_table[param.id()] = reference(&*current_arg, access_path(param.id()));
        ++current_arg;
    }
    
    symbol_table[plan.result().id()] = reference(&*current_arg, access_path(plan.result().id()));
    
    compile(plan.body(), env, ctx);
    
    // Epilog
    env.builder().CreateRetVoid();
}

void compile_entry_point(const function_declaration& plan, llvm_environment& env, std::size_t id)
{
    compilation_context ctx;

    compile(plan, env, ctx);

    // Prolog
    std::vector<llvm::Type*> param_types;

    param_types.push_back(llvm::PointerType::get(
        llvm::PointerType::get(llvm::Type::getInt8Ty(llvm::getGlobalContext()), 0), 0));

    llvm::FunctionType* FT = llvm::FunctionType::get(
        llvm::Type::getVoidTy(llvm::getGlobalContext()), param_types, false);

    std::string entry_point_name = "kubus_cpu_plan" + std::to_string(id);
    
    llvm::Function* kernel =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, entry_point_name, &env.module());

    for (std::size_t i = 0; i < kernel->arg_size(); ++i)
    {
        kernel->setDoesNotAlias(i + 1);
    }

    env.set_current_function(kernel);
    
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", kernel);
    env.builder().SetInsertPoint(BB);

    // unpack args
    std::size_t counter = 0;

    std::vector<llvm::Value*> arguments;

    auto add_param = [&](const variable_declaration& param) mutable
    {
        llvm::Type* param_type = env.map_kubus_type(param.var_type());

        llvm::Value* ptr_to_arg =
            env.builder().CreateConstInBoundsGEP1_64(&kernel->getArgumentList().front(), counter);

        auto arg = env.builder().CreateLoad(ptr_to_arg);
        arg->setMetadata("tbaa", env.get_tbaa_node(access_path()));

        llvm::Value* typed_arg =
            env.builder().CreateBitCast(arg, llvm::PointerType::get(param_type, 0));

        llvm::Value* data_ptr =
            env.builder().CreateConstInBoundsGEP2_32(typed_arg, 0, 0, "data_ptr");

        auto data = env.builder().CreateLoad(data_ptr);
        data->setMetadata("tbaa", env.get_tbaa_node(access_path(param.id())));

        // TODO: get alignement from the ABI
        env.builder().CreateCall2(
            env.get_assume_align(), data,
            llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), 32));

        arguments.push_back(typed_arg);
        ++counter;
    };

    for (const auto& param : plan.params())
    {
        add_param(param);
    }

    add_param(plan.result());

    // body

    env.builder().CreateCall(env.module().getFunction(plan.name()), arguments);

    // Epilog
    env.builder().CreateRetVoid();
    
    // Emit all other plans.
    while(auto plan = ctx.get_next_plan_to_compile())
    {
        compile(*plan, env, ctx);
    }
}

class cpu_plan
{
public:
    explicit cpu_plan(void* entry_) : entry_(entry_)
    {
    }

    cpu_plan(const cpu_plan&) = delete;
    cpu_plan& operator=(const cpu_plan&) = delete;

    void execute(const std::vector<void*>& args) const
    {
        using entry_t = void (*)(void* const*);

        auto entry = reinterpret_cast<entry_t>(entry_);

        entry(args.data());
    }

private:
    void* entry_;
};

std::unique_ptr<cpu_plan> compile(function_declaration entry_point, llvm::ExecutionEngine& engine)
{
    static std::size_t unique_id = 0;

    llvm_environment env;

    compile_entry_point(entry_point, env, unique_id);

    std::unique_ptr<llvm::Module> the_module = env.detach_module();

    llvm::verifyModule(*the_module);

    the_module->setDataLayout(engine.getDataLayout());

    //the_module->dump();
    //std::cout << std::endl;

    llvm::PassManagerBuilder pass_builder;
    pass_builder.OptLevel = 3;
    pass_builder.SLPVectorize = false;
    pass_builder.LoopVectorize = false;
    pass_builder.DisableUnrollLoops = true;
    pass_builder.Inliner = llvm::createFunctionInliningPass();

    llvm::FunctionPassManager fn_pass_man(the_module.get());
    llvm::PassManager pass_man;

    fn_pass_man.add(new llvm::DataLayoutPass());
    pass_man.add(new llvm::DataLayoutPass());

    engine.getTargetMachine()->addAnalysisPasses(fn_pass_man);
    engine.getTargetMachine()->addAnalysisPasses(pass_man);

    pass_builder.populateFunctionPassManager(fn_pass_man);
    pass_builder.populateModulePassManager(pass_man);

    fn_pass_man.doInitialization();

    for (auto& fn : *the_module)
    {
        fn_pass_man.run(fn);
    }

    fn_pass_man.doFinalization();

    pass_man.run(*the_module);

    //the_module->dump();
    //std::cout << std::endl;

    /*std::cout << "The assembler output:\n\n";

    llvm::raw_os_ostream m3log(std::cout);
    llvm::formatted_raw_ostream fm3log(m3log);

    llvm::PassManager pMPasses;
    // pMPasses.add(new llvm::DataLayoutPass(*engine.getDataLayout()));
    engine.getTargetMachine()->addPassesToEmitFile(pMPasses, fm3log,
                                                   llvm::TargetMachine::CGFT_AssemblyFile);
    pMPasses.run(*the_module);*/

    engine.finalizeObject();

    engine.addModule(std::move(the_module));

    return util::make_unique<cpu_plan>(reinterpret_cast<void*>(
        engine.getFunctionAddress("kubus_cpu_plan" + std::to_string(unique_id++))));
}

class cpu_plan_registry
{
public:
    plan register_plan(std::unique_ptr<cpu_plan> p, std::vector<intent> intents)
    {
        auto plan_handle = handle_fac_.create();

        plans_.emplace(plan_handle, std::move(p));

        return plan(plan_handle, std::move(intents));
    }

    const cpu_plan& lookup_plan(const plan& handle) const
    {
        return *plans_.at(handle.id());
    }

private:
    util::handle_factory handle_fac_;
    std::unordered_map<util::handle, std::unique_ptr<cpu_plan>> plans_;
};
}

class cpu_compiler : public compiler
{
public:
    explicit cpu_compiler(cpu_plan_registry& plan_registry_) : plan_registry_(&plan_registry_)
    {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        llvm::InitializeNativeTargetAsmParser();

        llvm::EngineBuilder builder(
            util::make_unique<llvm::Module>("dummy", llvm::getGlobalContext()));

        builder.setMCPU(llvm::sys::getHostCPUName());

        auto attrs = deduce_host_cpu_features();
        builder.setMAttrs(attrs);

        engine_ = std::unique_ptr<llvm::ExecutionEngine>(builder.create());

#if LLVM_USE_INTEL_JITEVENTS
        llvm::JITEventListener* vtuneProfiler =
            llvm::JITEventListener::createIntelJITEventListener();
        engine_->RegisterJITEventListener(vtuneProfiler);
#endif
    }

    virtual ~cpu_compiler() = default;

    plan compile_plan(const function_declaration& plan_decl) override
    {
        auto param_count = plan_decl.params().size();

        std::vector<intent> intents(param_count, intent::in);
        intents.push_back(intent::inout);

        auto compiled_plan = compile(plan_decl, *engine_);

        return plan_registry_->register_plan(std::move(compiled_plan), std::move(intents));
    }

private:
    std::unique_ptr<llvm::ExecutionEngine> engine_;
    cpu_plan_registry* plan_registry_;
};

class cpu_executor : public executor
{
public:
    cpu_executor(cpu_plan_registry& plan_registry_, local_address_space& addr_space_,
                 const abi_info& abi_)
    : plan_registry_(&plan_registry_), addr_space_(&addr_space_), abi_(&abi_), exec_stack_(4 * 1024)
    {
    }

    virtual ~cpu_executor() = default;

    hpx::lcos::future<void> execute_plan(const plan& executed_plan, execution_context ctx) override
    {
        const cpu_plan& executed_cpu_plan = plan_registry_->lookup_plan(executed_plan);

        std::vector<void*> plan_args;

        std::vector<std::shared_ptr<memory_block>> used_mem_blocks;

        for (const auto& arg : ctx.args())
        {
            plan_args.push_back(
                build_object_metadata(*arg, *addr_space_, *abi_, exec_stack_, used_mem_blocks));
        }

        return hpx::async([&executed_cpu_plan, plan_args, used_mem_blocks, this]
                          {
                              executed_cpu_plan.execute(plan_args);
                              exec_stack_.clear();
                          });
    }

private:
    cpu_plan_registry* plan_registry_;
    local_address_space* addr_space_;
    const abi_info* abi_;
    execution_stack exec_stack_;
};

class cpu_backend final : public backend
{
public:
    cpu_backend(const abi_info& abi_)
    : addr_space_(util::make_unique<local_address_space>(util::make_unique<cpu_allocator>())),
      obj_factory_(util::make_unique<cpu_object_factory>(addr_space_->get_allocator(), abi_)),
      compiler_(util::make_unique<cpu_compiler>(plan_registry_)),
      executor_(util::make_unique<cpu_executor>(plan_registry_, *addr_space_, abi_))
    {
        // use hwloc to obtain informations over all local CPUs
    }

    virtual ~cpu_backend() = default;

    std::string id() const override
    {
        return "kubus.cpu";
    }

    std::vector<executor*> executors() const override
    {
        return {executor_.get()};
    }

    compiler& get_compiler() const override
    {
        return *compiler_;
    }

    local_object_factory& local_factory() const override
    {
        return *obj_factory_;
    }

    local_address_space& address_space() const override
    {
        return *addr_space_;
    }

private:
    std::unique_ptr<local_address_space> addr_space_;
    std::unique_ptr<cpu_object_factory> obj_factory_;
    cpu_plan_registry plan_registry_;
    std::unique_ptr<cpu_compiler> compiler_;
    std::unique_ptr<cpu_executor> executor_;
};

std::unique_ptr<cpu_backend> the_cpu_backend;
std::once_flag cpu_backend_init_flag;

extern "C" backend* init_cpu_backend(const abi_info* abi)
{
    std::call_once(cpu_backend_init_flag, [&]
                   {
                       the_cpu_backend = std::make_unique<cpu_backend>(*abi);
                   });

    return the_cpu_backend.get();
}
}
}
