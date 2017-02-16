#include <qbb/qubus/jit/type_conversion.hpp>

#include <qbb/qubus/jit/compiler.hpp>

#include <qbb/qubus/jit/load_store.hpp>
#include <qbb/qubus/jit/entry_block_alloca.hpp>

#include <qbb/qubus/pattern/core.hpp>
#include <qbb/qubus/pattern/IR.hpp>

#include <qbb/qubus/IR/type_inference.hpp>

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>

namespace qubus
{
namespace jit
{
reference emit_type_conversion(const type& target_type, const expression& arg,
                               compiler& comp)
{
    using pattern::_;

    auto& env = comp.get_module().env();
    auto& ctx = comp.get_module().ctx();

    reference arg_value_ptr = comp.compile(arg);

    auto& builder = env.builder();

    llvm::Instruction* arg_value = load_from_ref(arg_value_ptr, env, ctx);

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
                                                  llvm::Type* complex_type = env.map_qubus_type(
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
                                                                  env.map_qubus_type(types::complex(types::double_()));

                                                          llvm::Value* result =
                                                                  llvm::ConstantAggregateZero::get(complex_type);

                                                          result = builder.CreateInsertValue(
                                                                  result,
                                                                  builder.CreateFPExt(
                                                                          arg_value, env.map_qubus_type(types::double_())),
                                                                  std::vector<unsigned>{0, 0});

                                                          return result;
                                                      })
                                               .case_(complex_t(pattern::float_t),
                                                      [&]
                                                      {
                                                          llvm::Type* complex_type =
                                                                  env.map_qubus_type(types::complex(types::float_()));

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
                                                                  arg_value, env.map_qubus_type(target_type));
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
                                                          env.map_qubus_type(target_type);

                                                  llvm::Value* result =
                                                          llvm::ConstantAggregateZero::get(complex_type);

                                                  result = builder.CreateInsertValue(
                                                          result, builder.CreateSIToFP(
                                                                  arg_value,
                                                                  env.map_qubus_type(subtype.get())),
                                                          std::vector<unsigned>{0, 0});

                                                  return result;
                                              })
                                       .case_(pattern::float_t || pattern::double_t,
                                              [&] (const type& self)
                                              {
                                                  return builder.CreateSIToFP(
                                                          arg_value, env.map_qubus_type(self));
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
                                                                  env.map_qubus_type(target_type);

                                                          llvm::Value* result = llvm::UndefValue::get(complex_type);

                                                          llvm::Value* arg_value_real = builder.CreateExtractValue(
                                                                  arg_value, std::vector<unsigned>{0, 0});
                                                          llvm::Value* arg_value_imag = builder.CreateExtractValue(
                                                                  arg_value, std::vector<unsigned>{0, 1});

                                                          result = builder.CreateInsertValue(
                                                                  result, builder.CreateFPExt(
                                                                          arg_value_real,
                                                                          env.map_qubus_type(types::double_())),
                                                                  std::vector<unsigned>{0, 0});

                                                          result = builder.CreateInsertValue(
                                                                  result, builder.CreateFPExt(
                                                                          arg_value_imag,
                                                                          env.map_qubus_type(types::double_())),
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

    reference result_var_ref(result_var, access_path(), target_type);

    store_to_ref(result_var_ref, result, env, ctx);

    return result_var_ref;
}
}
}