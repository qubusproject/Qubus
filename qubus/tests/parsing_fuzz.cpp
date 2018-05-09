#include <qubus/IR/module.hpp>
#include <qubus/IR/parsing.hpp>

#include <string>
#include <cstring>

#include <iostream>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    static_assert(sizeof(*Data) == sizeof(char));

    std::string code;
    code.reserve(Size);

    for (auto iter = Data, end = Data + Size; iter != end; ++iter)
    {
        char value;

        std::memcpy(&value, iter, sizeof(char));

        code.push_back(value);
    }

    try
    {
        qubus::parse_qir(code);
    }
    catch (...)
    {
    }

    return 0; // Non-zero return values are reserved for future use.
}
