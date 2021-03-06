#include <qubus/jit/compile.hpp>

#include <qubus/jit/compiler.hpp>

#include <qubus/jit/array_access.hpp>
#include <qubus/jit/control_flow.hpp>
#include <qubus/jit/entry_block_alloca.hpp>
#include <qubus/jit/load_store.hpp>
#include <qubus/jit/loops.hpp>
#include <qubus/jit/operators.hpp>
#include <qubus/jit/type_conversion.hpp>

#include <qubus/IR/qir.hpp>
#include <qubus/IR/type_inference.hpp>

#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <iterator>

namespace qubus
{
namespace jit
{

reference compile(const expression& expr, compiler& comp)
{
    auto& env = comp.get_module().env();
    auto& ctx = comp.get_module().ctx();

    using pattern::_;

    auto& symbol_table = ctx.symbol_table();

    pattern::variable<binary_op_tag> btag;
    pattern::variable<unary_op_tag> utag;
    pattern::variable<const expression&> a, b, c, d;
    pattern::variable<util::optional_ref<const expression>> opt_expr;
    pattern::variable<std::vector<std::reference_wrapper<const expression>>> expressions,
        expressions2, expressions3;

    pattern::variable<variable_declaration> idx;
    pattern::variable<variable_declaration> var;
    pattern::variable<const function&> plan;

    pattern::variable<util::index_t> ival;
    pattern::variable<float> fval;
    pattern::variable<double> dval;

    pattern::variable<std::string> name;

    pattern::variable<type> t;

    auto& builder = env.builder();

    auto m =
        pattern::make_matcher<expression, reference>()
            .case_(binary_operator(btag, a, b),
                   [&] { return emit_binary_operator(btag.get(), a.get(), b.get(), comp); })
            .case_(unary_operator(utag, a),
                   [&] { return emit_unary_operator(utag.get(), a.get(), comp); })
            .case_(for_(idx, a, b, c, d),
                   [&] {
                       ctx.enter_new_scope();

                       llvm::Type* size_type = env.map_qubus_type(types::integer());

                       auto increment_ptr = comp.compile(c.get());
                       auto increment_value = load_from_ref(increment_ptr, env, ctx);

                       llvm::Value* induction_var = create_entry_block_alloca(
                           env.get_current_function(), size_type, nullptr, "ind");

                       auto induction_var_ref =
                           reference(induction_var, access_path(), types::integer());

                       symbol_table[idx.get().id()] = induction_var_ref;

                       reference lower_bound_ptr = comp.compile(a.get());
                       reference upper_bound_ptr = comp.compile(b.get());

                       auto lower_bound = load_from_ref(lower_bound_ptr, env, ctx);
                       auto upper_bound = load_from_ref(upper_bound_ptr, env, ctx);

                       emit_loop(induction_var_ref, lower_bound, upper_bound, increment_value,
                                 [&]() { comp.compile(d.get()); }, env, ctx);

                       return reference();
                   })
            .case_(if_(a, b, opt_expr),
                   [&] {
                       ctx.enter_new_scope();

                       auto condition = comp.compile(a.get());

                       emit_if_else(condition, [&] { comp.compile(b.get()); },
                                    [&] {
                                        if (opt_expr.get())
                                        {
                                            comp.compile(*opt_expr.get());
                                        }
                                    },
                                    env, ctx);

                       return reference();
                   })
            .case_(compound(expressions),
                   [&] {
                       for (const auto& subexpr : expressions.get())
                       {
                           comp.compile(subexpr);
                       }

                       return reference();
                   })
            .case_(double_literal(dval),
                   [&] {
                       llvm::Type* double_type = env.map_qubus_type(types::double_());

                       llvm::Value* value = llvm::ConstantFP::get(double_type, dval.get());

                       auto var =
                           create_entry_block_alloca(env.get_current_function(), double_type);

                       reference var_ref(var, access_path(), types::double_());
                       store_to_ref(var_ref, value, env, ctx);

                       return var_ref;
                   })
            .case_(float_literal(fval),
                   [&] {
                       llvm::Type* float_type = env.map_qubus_type(types::float_());

                       llvm::Value* value = llvm::ConstantFP::get(float_type, fval.get());

                       llvm::Value* var =
                           create_entry_block_alloca(env.get_current_function(), float_type);

                       reference var_ref(var, access_path(), types::float_());
                       store_to_ref(var_ref, value, env, ctx);

                       return var_ref;
                   })
            .case_(integer_literal(ival),
                   [&] {
                       llvm::Type* size_type = env.map_qubus_type(types::integer());

                       llvm::Value* value = llvm::ConstantInt::get(size_type, ival.get());

                       llvm::Value* var =
                           create_entry_block_alloca(env.get_current_function(), size_type);

                       reference var_ref(var, access_path(), types::integer());
                       store_to_ref(var_ref, value, env, ctx);

                       return var_ref;
                   })
            .case_(integer_range(a, b, c),
                   [&] {
                       auto offset = comp.compile(a.get());
                       auto bound = comp.compile(b.get());
                       auto stride = comp.compile(c.get());

                       auto integer_range_type = env.map_qubus_type(types::integer_range);

                       auto result = create_entry_block_alloca(env.get_current_function(),
                                                               integer_range_type);

                       auto offset_ptr =
                           builder.CreateStructGEP(integer_range_type, result, 0, "offset_ptr");
                       auto bound_ptr =
                           builder.CreateStructGEP(integer_range_type, result, 1, "bound_ptr");
                       auto stride_ptr =
                           builder.CreateStructGEP(integer_range_type, result, 2, "stride_ptr");

                       builder.CreateStore(load_from_ref(offset, env, ctx), offset_ptr);
                       builder.CreateStore(load_from_ref(bound, env, ctx), bound_ptr);
                       builder.CreateStore(load_from_ref(stride, env, ctx), stride_ptr);

                       return reference(result, access_path(), types::integer_range);
                   })
            .case_(intrinsic_function_n(pattern::value("delta"), a, b),
                   [&] {
                       // TODO: Test if a and b are integers.

                       auto int_type = env.map_qubus_type(types::integer());

                       auto a_ref = comp.compile(a.get());
                       auto b_ref = comp.compile(b.get());

                       auto a_value = load_from_ref(a_ref, env, ctx);

                       auto b_value = load_from_ref(b_ref, env, ctx);

                       auto cond = builder.CreateICmpEQ(a_value, b_value);

                       auto one = llvm::ConstantInt::get(int_type, 1);
                       auto zero = llvm::ConstantInt::get(int_type, 0);

                       auto result_value = builder.CreateSelect(cond, one, zero);

                       auto result =
                           create_entry_block_alloca(env.get_current_function(), int_type);

                       reference result_ref(result, access_path(), types::integer());
                       store_to_ref(result_ref, result_value, env, ctx);

                       return result_ref;
                   })
            .case_(intrinsic_function_n(pattern::value("extent"), a, b),
                   [&] { return extent(a.get(), b.get(), comp); })
            .case_(intrinsic_function_n(pattern::value("min"), a, b),
                   [&] {
                       reference left_value_ptr = comp.compile(a.get());
                       reference right_value_ptr = comp.compile(b.get());

                       llvm::Instruction* left_value = load_from_ref(left_value_ptr, env, ctx);

                       llvm::Instruction* right_value = load_from_ref(right_value_ptr, env, ctx);

                       type result_type = typeof_(a.get());

                       auto cond = builder.CreateICmpSLT(left_value, right_value);

                       auto result = builder.CreateSelect(cond, left_value, right_value);

                       auto result_var = create_entry_block_alloca(env.get_current_function(),
                                                                   env.map_qubus_type(result_type));

                       reference result_ref(result_var, access_path(), result_type);
                       store_to_ref(result_ref, result, env, ctx);

                       return result_ref;
                   })
            .case_(intrinsic_function_n(pattern::value("max"), a, b),
                   [&] {
                       reference left_value_ptr = comp.compile(a.get());
                       reference right_value_ptr = comp.compile(b.get());

                       llvm::Instruction* left_value = load_from_ref(left_value_ptr, env, ctx);

                       llvm::Instruction* right_value = load_from_ref(right_value_ptr, env, ctx);

                       type result_type = typeof_(a.get());

                       auto cond = builder.CreateICmpSLT(left_value, right_value);

                       auto result = builder.CreateSelect(cond, right_value, left_value);

                       auto result_var = create_entry_block_alloca(env.get_current_function(),
                                                                   env.map_qubus_type(result_type));

                       reference result_ref(result_var, access_path(), result_type);
                       store_to_ref(result_ref, result, env, ctx);

                       return result_ref;
                   })
            .case_(intrinsic_function_n(pattern::value("select"), a, b, c),
                   [&] {
                       reference cond_value_ptr = comp.compile(a.get());
                       reference then_value_ptr = comp.compile(b.get());
                       reference else_value_ptr = comp.compile(c.get());

                       llvm::Instruction* cond_value = load_from_ref(cond_value_ptr, env, ctx);
                       llvm::Instruction* then_value = load_from_ref(then_value_ptr, env, ctx);
                       llvm::Instruction* else_value = load_from_ref(else_value_ptr, env, ctx);

                       auto result = builder.CreateSelect(cond_value, then_value, else_value);

                       type result_type = typeof_(a.get());

                       auto result_var = create_entry_block_alloca(env.get_current_function(),
                                                                   env.map_qubus_type(result_type));

                       reference result_ref(result_var, access_path(), result_type);
                       store_to_ref(result_ref, result, env, ctx);

                       return result_ref;
                   })
            .case_(intrinsic_function_n(pattern::value("delta"), a, b),
                   [&] {
                       // TODO: Test if a and b are integers.

                       auto int_type = env.map_qubus_type(types::integer());

                       auto a_ref = comp.compile(a.get());
                       auto b_ref = comp.compile(b.get());

                       auto a_value = load_from_ref(a_ref, env, ctx);

                       auto b_value = load_from_ref(b_ref, env, ctx);

                       auto cond = builder.CreateICmpEQ(a_value, b_value);

                       auto one = llvm::ConstantInt::get(int_type, 1);
                       auto zero = llvm::ConstantInt::get(int_type, 0);

                       auto result_value = builder.CreateSelect(cond, one, zero);

                       auto result =
                           create_entry_block_alloca(env.get_current_function(), int_type);

                       reference result_ref(result, access_path(), types::integer());
                       store_to_ref(result_ref, result_value, env, ctx);

                       return result_ref;
                   })
            .case_(type_conversion(t, a),
                   [&] { return emit_type_conversion(t.get(), a.get(), comp); })
            .case_(variable_ref(idx), [&] { return symbol_table.at(idx.get().id()); })
            .case_(subscription(a, expressions),
                   [&] { return emit_subscription(a.get(), expressions.get(), comp); })
            .case_(
                member_access(a, name),
                [&] {
                    auto obj_type = typeof_(a.get()).as<types::struct_>();

                    auto member_idx = obj_type.member_index(name.get());

                    auto obj_ref = comp.compile(a.get());

                    llvm::Value* indices[] = {builder.getInt32(0), builder.getInt32(0),
                                              builder.getInt32(member_idx)};

                    auto member = builder.CreateInBoundsGEP(env.map_qubus_type(obj_type),
                                                            obj_ref.addr(), indices);

                    auto member_type = obj_type[name.get()];

                    auto member_offset = builder.CreateLoad(member);

                    auto base_ptr =
                        builder.CreateBitCast(obj_ref.addr(), builder.getInt8PtrTy(), "base_ptr");

                    auto member_ptr =
                        builder.CreateInBoundsGEP(builder.getInt8Ty(), base_ptr, member_offset);

                    auto typed_member_ptr = builder.CreateBitCast(
                        member_ptr, env.map_qubus_type(member_type)->getPointerTo(0));

                    return reference(typed_member_ptr, obj_ref.origin() / name.get(), member_type);
                })
            .case_(
                local_variable_def(var, a),
                [&] {
                    auto var_type = env.map_qubus_type(var.get().var_type());

                    auto var_ptr = create_entry_block_alloca(env.get_current_function(), var_type);

                    auto init_value_ref = comp.compile(a.get());

                    auto init_value = load_from_ref(init_value_ref, env, ctx);

                    reference var_ref(var_ptr, access_path(var.get().id()), var.get().var_type());
                    store_to_ref(var_ref, init_value, env, ctx);

                    symbol_table[var.get().id()] = var_ref;

                    return reference();
                })
            .case_(construct(t, expressions), [&] {
                pattern::variable<type> value_type;

                auto m =
                    pattern::make_matcher<type, reference>()
                        .case_(
                            array_t(value_type, _),
                            [&](const type& self) {
                                auto size_type = env.map_qubus_type(types::integer());

                                const auto& args = expressions.get();

                                std::vector<util::index_t> extents;

                                try
                                {
                                    for (const auto& arg : args)
                                    {
                                        extents.push_back(
                                            arg.get().as<integer_literal_expr>().value());
                                    }
                                }
                                catch (const std::bad_cast&)
                                {
                                    throw 0;
                                }

                                std::size_t mem_size = 8;

                                for (auto extent : extents)
                                {
                                    mem_size *= extent;
                                }

                                llvm::Value* data_ptr;

                                if (mem_size < 256)
                                {
                                    llvm::Type* multi_array_type =
                                        env.map_qubus_type(value_type.get());

                                    std::size_t size = 1;

                                    for (auto extent : extents)
                                    {
                                        size *= extent;
                                    }

                                    multi_array_type = llvm::ArrayType::get(multi_array_type, size);

                                    data_ptr = builder.CreateConstInBoundsGEP2_64(
                                        create_entry_block_alloca(env.get_current_function(),
                                                                  multi_array_type),
                                        0, 0);
                                }
                                else
                                {
                                    std::vector<llvm::Value*> args;

                                    auto runtime =
                                        &*std::next(env.get_current_function()->arg_begin(),
                                                    env.get_current_function()->arg_size() - 1);

                                    args.push_back(runtime);
                                    args.push_back(llvm::ConstantInt::get(size_type, mem_size));

                                    data_ptr = builder.CreateBitCast(
                                        builder.CreateCall(env.get_alloc_scratch_mem(), args),
                                        env.map_qubus_type(value_type.get())->getPointerTo(0));

                                    ctx.get_current_scope().on_exit([args, &env, &builder] {
                                        builder.CreateCall(env.get_dealloc_scratch_mem(), args);
                                    });
                                }

                                auto shape_ptr = builder.CreateConstInBoundsGEP2_64(
                                    create_entry_block_alloca(
                                        env.get_current_function(),
                                        llvm::ArrayType::get(size_type, extents.size())),
                                    0, 0, "shape_ptr");

                                for (std::size_t i = 0; i < extents.size(); ++i)
                                {
                                    auto extent_ptr = builder.CreateConstInBoundsGEP1_64(
                                        shape_ptr, i, "extend_ptr");

                                    store_to_ref(
                                        reference(extent_ptr, access_path(), types::integer()),
                                        llvm::ConstantInt::get(size_type, extents[i]), env, ctx);
                                }

                                auto array_type = env.map_qubus_type(self);

                                auto array_ptr = create_entry_block_alloca(
                                    env.get_current_function(), array_type);

                                auto data_member_ptr =
                                    builder.CreateConstInBoundsGEP2_32(array_type, array_ptr, 0, 0);
                                store_to_ref(
                                    reference(data_member_ptr, access_path(), value_type.get()),
                                    data_ptr, env, ctx);

                                auto shape_member_ptr =
                                    builder.CreateConstInBoundsGEP2_32(array_type, array_ptr, 0, 1);
                                store_to_ref(
                                    reference(shape_member_ptr, access_path(), types::integer()),
                                    shape_ptr, env, ctx);

                                return reference(array_ptr, access_path(), self);
                            })
                        .case_(value_type, [&] {
                            if (!expressions.get().empty())
                                throw 0;

                            auto var_ptr = create_entry_block_alloca(
                                env.get_current_function(), env.map_qubus_type(value_type.get()));

                            return reference(var_ptr, access_path(), value_type.get());
                        });

                return pattern::match(t.get(), m);
            });

    auto result = pattern::match(expr, m);

    auto m2 = pattern::make_matcher<expression, void>().case_(variable_scope(_, _),
                                                              [&] { ctx.exit_current_scope(); });

    pattern::try_match(expr, m2);

    return result;
}
} // namespace jit
} // namespace qubus