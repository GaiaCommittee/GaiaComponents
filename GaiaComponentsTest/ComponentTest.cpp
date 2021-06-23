#include <gtest/gtest.h>
#include <iostream>
#include "../GaiaComponents/GaiaComponents.hpp"

using namespace Gaia::Components;

class SampleBasicComponent : public Component
{
protected:
    void OnAttachedToComponent() override
    {
        std::cout << "Attached to a component " << GetParent() << std::endl;
    }

    void OnDetachedFromComponent() override
    {
        std::cout << "Detached from a component " << GetParent() << std::endl;
    }

    void OnComponentAttached(Component *component) override
    {
        std::cout << "A component attached " << component << std::endl;
    }

    void OnComponentDetached(Component *component) override
    {
        std::cout << "A component detached " << component << std::endl;
    }
};

class SampleValueComponent : public Component
{
public:
    int SampleValue {0};

    SampleValueComponent() = default;
    explicit SampleValueComponent(int value) : SampleValue(value)
    {}
};

TEST(ComponentTest, Basic)
{
    SampleBasicComponent sample_basic_component;
    sample_basic_component.AddComponent<SampleValueComponent>(1);

    EXPECT_NE(sample_basic_component.GetComponent<SampleValueComponent>(), nullptr);
    EXPECT_EQ(sample_basic_component.GetComponent<SampleBasicComponent>(), nullptr);

    EXPECT_EQ(sample_basic_component.GetComponent<SampleValueComponent>()->SampleValue, 1);

    sample_basic_component.AddComponent<SampleBasicComponent>();
    sample_basic_component.RemoveComponent<SampleValueComponent>();

    EXPECT_EQ(sample_basic_component.GetComponent<SampleValueComponent>(), nullptr);
    EXPECT_NE(sample_basic_component.GetComponent<SampleBasicComponent>(), nullptr);

    auto sample_sub_component = sample_basic_component.SeparateComponent<SampleBasicComponent>();
    EXPECT_NE(sample_sub_component.get(), nullptr);
    auto sample_sub_component2 = sample_basic_component.SeparateComponent<SampleBasicComponent>();
    EXPECT_EQ(sample_sub_component2.get(), nullptr);

    auto sample_value_component = sample_basic_component.AddComponent<SampleValueComponent>(3);
    sample_value_component->SampleValue = 6;
    EXPECT_EQ(sample_basic_component.GetComponent<SampleValueComponent>()->SampleValue, 6);
    auto sample_value_component_instance = sample_basic_component.SeparateComponent<SampleValueComponent>();
    EXPECT_EQ(sample_value_component->SampleValue, 6);
    sample_value_component->SampleValue = 7;
    sample_basic_component.AdoptComponent<SampleValueComponent>(std::move(sample_value_component_instance));
}