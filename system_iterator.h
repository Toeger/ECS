#ifndef SYSTEMITERATOR_H
#define SYSTEMITERATOR_H

#include "entity_handle.h"
#include "system_base.h"
#include "utility/asserts.h"

#include <iostream>

namespace ECS {
	/*
	Iterator over Entitys with specifiable components.
	Iterating over Entitys with some specific components goes through all entities with any of the components
	TODO: It would be more ideomatic but less efficient to use begin and end style iterators.
	TODO: It would make sense to have a get function that returns a tuple of components. For that the struct layout (?) needs to be changed.
	TODO: Add casting/converting iterators. Removing a component would be fairly easy, adding a component would initiate searching.
		  Unrelated iterators don't really make sense.
	TODO: Replace .at with []
	*/
	template <class First, class... Rest>
	struct System_iterator {
		void advance() {
			assert_fast(System::get_ids<First>().at(current_indexes[0]) != Impl::max_id);
			advance(System::get_ids<First>().at(current_indexes[0]) + 1);
		}
		void advance(Impl::Id target) {
			if constexpr (sizeof...(Rest) == 0) {
				get_advanced_index(target);
			} else {
				auto new_target = get_advanced_index(target);
				while (new_target != target && new_target != Impl::max_id) {
					target = new_target;
					new_target = get_advanced_index(target);
				}
			}
		}
		operator bool() const {
			return System::get_ids<First>().at(current_indexes[0]) != Impl::max_id;
		}
		auto &operator++() {
			advance();
			return *this;
		}
		template <class U>
		auto &get() {
			using typelist = Utility::Type_list<First, Rest...>;
			constexpr auto index = typelist::template get_index<U>();
			std::cout << System::get_components<U>().size() << '\n';
			std::cout << Utility::type_name<U>() << '\n' << std::flush;
			return System::get_components<U>().at(current_indexes[index]);
		}
		auto get_ids() const {
			std::array<std::size_t, sizeof...(Rest) + 1> ids;
			get_ids(ids);
			return ids;
		}
		ECS::Entity_handle get_entity_handle() const {
			return Entity_handle{current_indexes[0]};
		}

		private:
		template <int index = 0>
		auto get_advanced_index(Impl::Id target) {
			using typelist = Utility::Type_list<First, Rest...>;
			using index_type = typename typelist::template nth<index>;
			auto &ids = System::get_ids<index_type>();
			std::cout << ids.size() << '\n';
			for (const auto &id : ids) {
				std::cout << id << ' ';
			}
			std::cout << '\n' << std::flush;
			while (ids.at(current_indexes[index]) < target) {
				current_indexes[index]++;
			}
			if constexpr (index + 1 < typelist::size) {
				return std::max(current_indexes[index], get_advanced_index<index + 1>(current_indexes[index]));
			}
			return ids.at(current_indexes[index]);
		}
		template <std::size_t index = 0>
		void get_ids(std::array<std::size_t, sizeof...(Rest) + 1> &ids) {
			using typelist = Utility::Type_list<First, Rest...>;
			ids[index] = System::get_ids<typelist::nth<index>>(current_indexes[index]);
			if constexpr (index + 1 < ids.size()) {
				get_ids<index + 1>(ids);
			}
		}
		std::array<std::size_t, sizeof...(Rest) + 1> current_indexes{};
	};

	//comparison functions
	template <class... T>
	bool operator==(const System_iterator<T...> &lhs, const System_iterator<T...> &rhs) {
		return lhs.current_index == rhs.current_index; //TODO: we don't check other indexes?
	}
	template <class... T>
	bool operator<(const System_iterator<T...> &lhs, std::nullptr_t) {
		return lhs.current_index != Impl::max_id;
	}
} // namespace ECS

#endif // SYSTEMITERATOR_H
