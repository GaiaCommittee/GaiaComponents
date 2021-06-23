#include "Component.hpp"

namespace Gaia::Components
{
    /// Default implementation for being attached event.
    void Component::OnAttachedToComponent()
    {}
    /// Default implementation for being detached event.
    void Component::OnDetachedFromComponent()
    {}
    /// Default implementation for sub component attaching event.
    void Component::OnComponentAttached(Component *component)
    {}
    /// Default implementation for sub component detaching event.
    void Component::OnComponentDetached(Component *component)
    {}

    /// Add a sub component_instance to this component_instance.
    Component* Component::AddSubComponent(std::size_t hash, std::unique_ptr<Component>&& component_instance)
    {
        Component* component_pointer = component_instance.get();

        std::unique_lock lock(SubComponentsMutex);

        auto finder = SubComponents.find(hash);
        if (finder != SubComponents.end())
        {
            OnComponentDetached(finder->second.get());
            finder->second->OnDetachedFromComponent();
            finder->second = std::move(component_instance);
        }
        else
        {
            SubComponents.emplace(hash, std::move(component_instance));
        }

        component_pointer->Parent = this;
        OnComponentAttached(component_pointer);
        component_pointer->OnAttachedToComponent();

        return component_pointer;
    }

    /// Remove the sub component with the demanded hash code.
    void Component::RemoveSubComponent(std::size_t hash)
    {
        std::unique_lock lock(SubComponentsMutex);

        auto finder = SubComponents.find(hash);
        if (finder != SubComponents.end())
        {
            finder->second->OnDetachedFromComponent();
            OnComponentDetached(finder->second.get());
            SubComponents.erase(finder);
        }
    }

    /// Get the sub component with the demanded hash code.
    Component* Component::GetSubComponent(std::size_t hash)
    {
        std::shared_lock lock(SubComponentsMutex);

        auto finder = SubComponents.find(hash);
        if (finder != SubComponents.end())
        {
            return finder->second.get();
        }
        return nullptr;
    }

    /// Destructor which will invoke OnDetachedFromComponent() for all existing sub components.
    Component::~Component()
    {
        for (auto& component : SubComponents)
        {
            component.second->OnDetachedFromComponent();
        }
    }

    /// Separate a sub component.
    std::unique_ptr<Component> Component::SeparateSubComponent(std::size_t hash)
    {
        std::shared_lock lock(SubComponentsMutex);

        auto finder = SubComponents.find(hash);
        if (finder != SubComponents.end())
        {
            auto component = std::move(finder->second);
            SubComponents.erase(finder);
            return component;
        }
        return std::unique_ptr<Component>();
    }
}