#ifndef SYSTEM_H
#define SYSTEM_H

#include "entity_handle.h"
#include "system_base.h"
#include "system_iterator.h"

//get an Entity_handle for an Entity that owns the given component
//the given component must be owned by an Entity, otherwise it is UB!
template <class Component>
ECS::Entity_handle ECS::System::component_to_entity_handle(const Component &component) {
	auto &components = get_components<Component>();
	assert_fast(components.size());
	assert_fast(&components.front() <= &component);
	assert_fast(&components.back() >= &component);
	auto index = &component - &components.front();
	return Entity_handle{get_ids<Component>()[index]};
}

template <class... Components>
ECS::System_iterator<Components...> ECS::System::range() {
	System_iterator<Components...> si;
	si.advance(0);
	return si;
}

template <class... Components>
ECS::System::Range<Components...> ECS::System::get_range() {
	return {};
}

template <class... Components>
ECS::System_iterator<Components...> ECS::System::Range<Components...>::begin() {
	System_iterator<Components...> si;
	si.advance(0);
	return si;
}

template <class... Components>
std::nullptr_t ECS::System::Range<Components...>::end() {
	return nullptr;
}

#endif // SYSTEM_H
