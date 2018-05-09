#ifndef QUBUS_COMPONENT_REF_HPP
#define QUBUS_COMPONENT_REF_HPP

namespace qubus
{

template <typename ComponentClient>
class component_ref
{
public:


    ~component_ref()
    {
        client_.finalize();
    }
private:
    ComponentClient client_;
};

}

#endif
