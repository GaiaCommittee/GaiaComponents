#pragma once

#include <memory>
#include <unordered_map>
#include <shared_mutex>
#include <typeindex>
#include <type_traits>

namespace Gaia::Components
{
    /**
     * @brief Component is both the declaration of the support to a specular kind of functions,
     *        and the interface to access those functions.
     * @details
     *  The great advantage of Component is that user can use it to modify the functions of the existing classes
     *  without deriving new classes from them and functions of objects of classes derived from Component can be
     *  dynamically confirmed, reorganized and modified.
     */
    class Component
    {
    private:
        /// Mutex for sub components map.
        std::shared_mutex SubComponentsMutex;
        /// Map type hash code to sub component instance.
        std::unordered_map<std::size_t, std::unique_ptr<Component>> SubComponents;

        /**
         * @brief Add a sub component to this component_instance.
         * @param hash The hash code of the component to add.
         * @param component The instance of the component to add.
         * @return The pointer to the newly added component.
         * @details Previous component with the same hash code will be replaced if it exist.
         */
        Component* AddSubComponent(std::size_t hash, std::unique_ptr<Component>&& component);
        /**
         * @brief Remove the sub component with the demanded hash code.
         * @param hash The hash code of the component to remove.
         * @details This function will do nothing if the component with the given hash does not exist.
         */
        void RemoveSubComponent(std::size_t hash);
        /**
         * @brief Get the sub component with the demanded hash code.
         * @param hash The hash code of the component to separate.
         * @return The pointer to the sub component with the given hash code or nullptr if it does not exist.
         */
        Component* GetSubComponent(std::size_t hash);
        /**
         * @brief Separate a sub component into a individual component.
         * @param hash The hash code of the component to separate.
         * @return The instance of the separated component.
         */
        std::unique_ptr<Component> SeparateSubComponent(std::size_t hash);

        /// Pointer to the parent component.
        Component* Parent {nullptr};

    protected:

        /**
         * @brief Get the pointer to the parent component instance.
         * @tparam ComponentType The type of parent component to convert the pointer into.
         * @return The pointer of the parent component.
         */
        template <typename ComponentType = Component>
        ComponentType* GetParent()
        {
            static_assert(std::is_base_of_v<Component, ComponentType>,
                          "ComponentType must be derived from Component.");
            return static_cast<ComponentType*>(Parent);
        }

        /**
         * @brief Triggered when this component is added to a parent component.
         * @details Different from the constructor, that this function will only be invoked if this component
         *          is used as a sub component rather a independent component.
         *          When this function is invoked, GetParent() has already been able to correctly return the
         *          pointer to the parent component.
         *          This function will do nothing by default.
         */
        virtual void OnAttachedToComponent();
        /**
         * @brief Triggered when this component is removed from a parent component.
         * @details Different from the destructor, that this function will only be invoked if this component
         *          is used as a sub component rather a independent component.
         *          When this function is invoked, GetParent() is still able to correctly return the
         *          pointer to the parent component.
         *          This function will do nothing by default.
         */
        virtual void OnDetachedFromComponent();
        /**
         * @brief Triggered when a sub component is added to this component.
         * @details This function will be invoked before the OnAttachedToComponent() of the sub component.
         */
        virtual void OnComponentAttached(Component* component);
        /**
         * @brief Triggered when a sub component is removed from this component.
         * @details This function will be invoked after the OnDetachedFromComponent() of the sub component.
         */
        virtual void OnComponentDetached(Component* component);

    public:
        /// Destructor which will invoke OnDetachedFromComponent() for all existing sub components.
        virtual ~Component();

        /// Get all sub components of this component.
        [[nodiscard]] const decltype(SubComponents)& GetComponents() const noexcept
        {
            return SubComponents;
        }

        /**
         * @brief Check whether this component has the sub component of the given type or not.
         * @tparam ComponentType Type of sub component.
         * @retval true This component has a sub component of the given type.
         * @retval false This component does not have a sub component of the given type.
         */
        template <typename ComponentType>
        bool HasComponent()
        {
            return GetSubComponent(typeid(ComponentType).hash_code()) != nullptr;
        }

        /**
         * @brief Add a sub component to this component.
         * @tparam ComponentType The type of the component to construct and add.
         * @tparam ConstructorArguments The types of arguments to pass to the sub component constructor.
         * @param arguments Arguments to pass to the sub component constructor.
         * @return The pointer to the newly added component.
         * @details Previous component with the same type will be replaced if it exist.
         */
        template <typename ComponentType, typename... ConstructorArguments>
        ComponentType* AddComponent(ConstructorArguments... arguments)
        {
            static_assert(std::is_base_of_v<Component, ComponentType>,
                    "ComponentType must be derived from Component.");
            return dynamic_cast<ComponentType*>(
                    AddSubComponent(typeid(ComponentType).hash_code(),
                                    std::make_unique<ComponentType>(arguments...)));
        }

        /**
         * @brief Adopt a component instance to this component.
         * @tparam ComponentType The type of the component to adopt and add.
         * @param component The instance of the component to adopt and add.
         * @return The pointer to this component.
         * @details Previous component with the same type will be replaced if it exist.
         */
        template <typename ComponentType>
        ComponentType* AdoptComponent(std::unique_ptr<ComponentType>&& component)
        {
            static_assert(std::is_base_of_v<Component, ComponentType>,
                          "ComponentType must be derived from Component.");
            auto component_instance = std::unique_ptr<Component>(
                    dynamic_cast<Component*>(component.release()));
            return dynamic_cast<ComponentType*>(
                    AddSubComponent(typeid(ComponentType).hash_code(),
                                    std::move(component_instance)));
        }

        /**
         * @brief Remove the sub component of the given type.
         * @tparam ComponentType The type of the component to remove.
         */
        template <typename ComponentType>
        void RemoveComponent()
        {
            static_assert(std::is_base_of_v<Component, ComponentType>,
                          "ComponentType must be derived from Component.");
            RemoveSubComponent(typeid(ComponentType).hash_code());
        }

        /**
         * @brief Get the component instance of the given type.
         * @tparam ComponentType The type of the component to get.
         * @return The instance of the given component type,
         *         or nullptr if the sub component with the given type does not exist.
         */
        template <typename ComponentType>
        ComponentType* GetComponent()
        {
            static_assert(std::is_base_of_v<Component, ComponentType>,
                          "ComponentType must be derived from Component.");
            return dynamic_cast<ComponentType*>(GetSubComponent(typeid(ComponentType).hash_code()));
        }

        /**
         * @brief Get or create the component if it does not exist.
         * @tparam ComponentType Component type to acquire.
         * @return Pointer to the desired component.
         */
        template <typename ComponentType>
        ComponentType* AcquireComponent()
        {
            static_assert(std::is_base_of_v<Component, ComponentType>,
                          "ComponentType must be derived from Component.");
            auto* component = GetComponent<ComponentType>();
            if (component) return component;
            return AddComponent<ComponentType>();
        }

        /**
         * @brief Separate a sub component into a individual component.
         * @tparam ComponentType The type of the component to separate into a independent component.
         * @return The instance of the component.
         */
        template <typename ComponentType>
        std::unique_ptr<ComponentType> SeparateComponent()
        {
            static_assert(std::is_base_of_v<Component, ComponentType>,
                          "ComponentType must be derived from Component.");
            return std::unique_ptr<ComponentType>(
                    dynamic_cast<ComponentType*>(
                            SeparateSubComponent(typeid(ComponentType).hash_code()).release()));
        }
    };
}