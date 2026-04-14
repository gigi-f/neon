#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>

using Entity = uint32_t;
const Entity MAX_ENTITIES = 100000;

class Registry {
public:
    Entity create() {
        return next_entity++;
    }

    template<typename T, typename... Args>
    T& assign(Entity entity, Args&&... args) {
        auto type = std::type_index(typeid(T));
        if (components.find(type) == components.end()) {
            components[type] = std::make_unique<ComponentArray<T>>();
        }
        return get_array<T>()->insert(entity, T{std::forward<Args>(args)...});
    }

    template<typename T>
    T& get(Entity entity) {
        return get_array<T>()->get(entity);
    }

    template<typename T>
    bool has(Entity entity) {
        auto type = std::type_index(typeid(T));
        if (components.find(type) == components.end()) return false;
        return get_array<T>()->has(entity);
    }

    template<typename... ComponentTypes>
    std::vector<Entity> view() {
        std::vector<Entity> result;
        for (Entity e = 0; e < next_entity; ++e) {
            if ((has<ComponentTypes>(e) && ...)) {
                result.push_back(e);
            }
        }
        return result;
    }

private:
    Entity next_entity = 0;

    struct IComponentArray {
        virtual ~IComponentArray() = default;
    };

    template<typename T>
    struct ComponentArray : public IComponentArray {
        std::unordered_map<Entity, T> data;
        T& insert(Entity e, T component) {
            data[e] = component;
            return data[e];
        }
        T& get(Entity e) { return data.at(e); }
        bool has(Entity e) const { return data.find(e) != data.end(); }
    };

    std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> components;

    template<typename T>
    ComponentArray<T>* get_array() {
        return static_cast<ComponentArray<T>*>(components[std::type_index(typeid(T))].get());
    }
};
