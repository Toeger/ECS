#ifndef SYSTEM_BASE_H
#define SYSTEM_BASE_H

#include "ecs_impl.h"
#include "utility.h"

#include <atomic>
#include <functional>
#include <type_traits>
#include <vector>

namespace ECS {
	template <class H, class... T>
	struct System_iterator;
	struct Entity_handle;
	/*
	System keeps the components of all Entitys in a vector per component type and allows to iterate over Entitys with specified components.
	You only use System to iterate. Use Entities to add components.
	Limitations:
		Cannot have multiple components of the same type in one Entity. You can get around that with an array or vector of components.
	*/
	struct System {
		template <class Component>
		static std::vector<Utility::remove_cvr<Component>> &get_components() {
			return components<Utility::remove_cvr<Component>>;
		}
		template <class Component>
		static std::vector<Impl::Id> &get_ids() {
			return ids<Utility::remove_cvr<Component>>;
		}
		//get a range iterator for a list of components, range<Position, Direction> iterates over all Entities with both a Position and a Direction component
		template <class... Components>
		static System_iterator<Components...> range();
		//get entity handle from a component that has been added to an entity
		template <class Component>
		static Entity_handle component_to_entity_handle(const Component &component);
		//run all systems
		static void run_systems() {
			for (auto &f : function_pointer_systems) {
				f();
			}
			for (auto &f : function_systems) {
				f();
			}
		}
		//add a system
		template <class... Components, class Function>
		static void add_system(Function &&f) {
			add_to_system([f = std::move(f)] {
				for (auto sit = range<Components...>(); sit; sit.advance()) {
					f(sit.get_entity_handle());
				}
			});
		}
		//add a system which computes something once for all entities
		template <class... Components, class Function, class PrecomputeFunction>
		static void add_system(Function &&f, PrecomputeFunction &&pf) {
			add_to_system([ f = std::move(f), pf = std::move(pf) ] {
				auto pc = pf();
				for (auto sit = range<Components...>(); sit; sit.advance()) {
					f(sit.get_entity_handle(), pc);
				}
			});
		}

		//add a system which doesn't loop through components
		template <class Function>
		static void add_independent_system(Function &&f) {
			add_to_system(std::forward<Function>(f));
		}

		private:
#ifndef NDEBUG
		//in debug mode increment the component_state whenever components are removed or added to notice when iterators become invalid
		template <class T>
		static std::atomic<unsigned> component_state;
		template <class H, class... T>
		friend struct ECS::System_iterator;
#endif

		//vector to store the components
		template <class Component>
		static std::vector<Component> components;
		//vector to store the IDs. ids and components are locked, so components<CTYPE>[x] is the component that belongs to entity ids<CTYPE>[x]
		template <class Component>
		static std::vector<Impl::Id> ids;
		/* TODO: could make components and ids use the same memory since they reallocate at the same time, but this only saves a few memory allocations
		   and is probably not worth it */

		template <class Function>
		static std::enable_if_t<std::is_convertible<Function, void (*)()>::value> add_to_system(Function &&f) {
			function_pointer_systems.push_back(std::forward<Function>(f));
		}
		template <class Function>
		static std::enable_if_t<!std::is_convertible<Function, void (*)()>::value> add_to_system(Function &&f) {
			function_systems.push_back(std::forward<Function>(f));
		}
		static std::vector<void (*)()> function_pointer_systems;
		static std::vector<std::function<void()>> function_systems;
	};
#ifndef NDEBUG
	template <class T>
	static std::atomic<unsigned> component_state;
#endif
	template <class Component>
	std::vector<Component> ECS::System::components{};
	template <class Component>
	std::vector<ECS::Impl::Id> ECS::System::ids{ECS::Impl::max_id};
} // namespace ECS

#endif // SYSTEM_BASE_H
