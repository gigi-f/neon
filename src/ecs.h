#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <typeindex>
#include <algorithm>

using Entity = uint32_t;
const Entity MAX_ENTITIES = 100000;

class Registry {
public:
    Entity create() {
        if (!free_list.empty()) {
            Entity e = free_list.back();
            free_list.pop_back();
            live.insert(e);
            return e;
        }
        Entity e = next_entity++;
        live.insert(e);
        return e;
    }

    void destroy(Entity e) {
        if (!live.count(e)) return;
        live.erase(e);
        for (auto& [type, arr] : components) {
            arr->remove(e);
        }
        free_list.push_back(e);
    }

    bool alive(Entity e) const {
        return live.count(e) > 0;
    }

    size_t entity_count() const {
        return live.size();
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

    // Returns all live entities that have every listed component type.
    // Iterates the smallest matching component array — O(m * k) where
    // m = size of smallest set, k = number of queried types.
    template<typename... ComponentTypes>
    std::vector<Entity> view() {
        std::vector<Entity> result;

        // Collect all arrays; any missing type means zero results.
        IComponentArray* arrays[] = { get_or_null<ComponentTypes>()... };
        IComponentArray* smallest = nullptr;
        for (auto* arr : arrays) {
            if (!arr) return result;
            if (!smallest || arr->size() < smallest->size()) smallest = arr;
        }
        if (!smallest) return result;

        for (Entity e : smallest->keys()) {
            if (!live.count(e)) continue;
            if ((has<ComponentTypes>(e) && ...)) {
                result.push_back(e);
            }
        }
        return result;
    }

private:
    Entity next_entity = 0;
    std::unordered_set<Entity> live;
    std::vector<Entity> free_list;

    struct IComponentArray {
        virtual ~IComponentArray() = default;
        virtual void remove(Entity e) = 0;
        virtual size_t size() const = 0;
        virtual std::vector<Entity> keys() const = 0;
    };

    template<typename T>
    struct ComponentArray : public IComponentArray {
        std::unordered_map<Entity, T> data;

        T& insert(Entity e, T component) {
            data[e] = std::move(component);
            return data[e];
        }
        T& get(Entity e) { return data.at(e); }
        bool has(Entity e) const { return data.find(e) != data.end(); }
        void remove(Entity e) override { data.erase(e); }
        size_t size() const override { return data.size(); }
        std::vector<Entity> keys() const override {
            std::vector<Entity> result;
            result.reserve(data.size());
            for (auto& [e, _] : data) result.push_back(e);
            return result;
        }
    };

    std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> components;

    template<typename T>
    ComponentArray<T>* get_array() {
        return static_cast<ComponentArray<T>*>(components[std::type_index(typeid(T))].get());
    }

    template<typename T>
    IComponentArray* get_or_null() {
        auto it = components.find(std::type_index(typeid(T)));
        return (it != components.end()) ? it->second.get() : nullptr;
    }

    static IComponentArray* smaller_of(IComponentArray* a, IComponentArray* b) {
        if (!b) return a;
        if (!a) return b;
        return (a->size() <= b->size()) ? a : b;
    }
};
