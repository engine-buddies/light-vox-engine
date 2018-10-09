#include "stdafx.h"
#include "detail.h"
#include <limits>
#include <cstdint>
#include <vector>
#include <assert.h>

template <class T>
struct Component;

struct Entity {
    Entity()
        : id(_id_count++) // For demo only. Id == position in component
                          // buffer.
    {
    }

    template <class T>
    Component<T> add_component() {
        static_assert(std::is_base_of<Component<T>, T>::value,
            "Your component needs to inherit Component<>.");

        /* Don't allow duplicate components. */
        if (auto ret = get_component<T>())
            return ret;

        return Component<T>::add_component(*this);
    }

    template <class T>
    Component<T> get_component() {
        static_assert(std::is_base_of<Component<T>, T>::value,
            "Components must inherit Component<>.");

        return Component<T>{ *this };
    }

    uint32_t id;
    static const Entity dummy;

private:
    Entity(uint32_t id_)
        : id(id_) {
    }

    static uint32_t _id_count;
};


template <class T>
struct Component {
    Component(Entity e = Entity::dummy)
        : entity(e) {
    }

    static void* operator new(size_t) = delete;
    static void* operator new[](size_t) = delete;

    operator bool() const {
        return entity.id < _components.size();
    }

    T* operator->() const {
        assert(*this == true && "Component doesn't exist.");
        return &_components[entity.id];
    }

    template <class U>
    Component<U> add_component() {
        static_assert(std::is_base_of<Component<U>, U>::value,
            "Components must inherit Component<>.");
        return entity.add_component<U>();
    }

    template <class U>
    Component<U> get_component() {
        static_assert(std::is_base_of<Component<U>, U>::value,
            "Components must inherit Component<>.");
        return Component<U>{ entity };
    }

    static Component<T> add_component(Entity e) {
        //      printf("Constructing %s Component. Entity : %u",
        // typeid(T).name(),                e.id);

        T t;
        t.entity = e;
        _components.emplace_back(std::move(t));

        if constexpr (is_detected_v<detail::has_init, T>) {
            _components.back().init();
        }

        return Component<T>{ e };
    }

    static void update_components(float dt) {
        if constexpr (is_detected_v<detail::has_update, T>) {
            for (size_t i = 0; i < _components.size(); ++i) {
                _components[i].update(dt);
            }
        }
    }

protected:
    Entity entity;

private:
    static std::vector<T> _components;
};
template <class T>
std::vector<T> Component<T>::_components = {};