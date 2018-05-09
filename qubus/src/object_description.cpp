#include <qubus/object_description.hpp>

namespace qubus
{

type compute_type(const object_description& desc)
{
    if (auto array_desc = desc.try_as<array_description>())
    {
        return types::array(array_desc->value_type(), array_desc->rank());
    }

    if (auto scalar_desc = desc.try_as<scalar_description>())
    {
        return scalar_desc->value_type();
    }

    if (auto struct_desc = desc.try_as<struct_description>())
    {
        return struct_desc->type();
    }

    throw 0;
}

namespace
{

object_layout compute_layout(const object_description& desc, long int& current_position, const abi_info& abi)
{
    if (auto array_desc = desc.try_as<array_description>())
    {
        auto array_layout = abi.get_array_layout(array_desc->value_type(), array_desc->shape());

        auto alignment = array_layout.alignment();

        auto position = current_position;
        auto size = array_layout.size();

        current_position += size;

        return object_layout(desc, position, size);
    }

    if (auto scalar_desc = desc.try_as<scalar_description>())
    {
        auto alignment = abi.get_align_of(scalar_desc->value_type());

        auto position = current_position;
        auto size = abi.get_size_of(scalar_desc->value_type());

        current_position += size;

        return object_layout(desc, position, size);
    }

    if (auto struct_desc = desc.try_as<struct_description>())
    {
        auto position = current_position;
        auto size = util::integer_cast<long int>(struct_desc->members().size() * sizeof(long int));

        current_position += size;

        std::vector<object_layout> partitions;

        for (const auto& member : struct_desc->members())
        {
            partitions.push_back(compute_layout(member.description, current_position, abi));
        }

        return object_layout(desc, position, size, std::move(partitions));
    }

    throw 0;
}

}

object_layout compute_layout(const object_description& desc, const abi_info& abi)
{
    long int position = 0;

    return compute_layout(desc, position, abi);
}

}