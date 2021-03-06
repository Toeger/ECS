#ifndef ENTITY_BASE_H
#define ENTITY_BASE_H

#include "ecs/log.h"
#include "ecs_impl.h"
#include "system_base.h"
#include "utility/asserts.h"

#include <algorithm>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <vector>

namespace ECS {
	//an Entity can have any type of component added to it
	//note that you cannot add multiple components of the same type, use vector<component> or array<component> to get around that

	namespace Impl {
		struct Entity_base {
			Entity_base(Impl::Id_t id)
				: id(id) {}
			Entity_base(Entity_base &&other) noexcept
				: id(other.id) {
				other.id = max_id;
			}
			Entity_base &operator=(Entity_base &&other) noexcept {
				std::swap(id, other.id);
				return *this;
			}
			~Entity_base() = default;

			//emplace a component into an Entity
			template <class Component, class... Args>
			Component &emplace(Args &&... args) {
				auto &ids = System::get_ids<Component>();
				auto &components = System::get_components<Component>();
				auto insert_position = std::lower_bound(begin(ids), end(ids), id);
				assert_fast(*insert_position != id); //disallow multiple components of the same type for the same entity
				typename std::remove_reference_t<decltype(components)>::iterator inserted_component; //TODO: find a way to make this prettier
				if constexpr (std::is_pod<Component>::value) {
					inserted_component = components.insert(begin(components) + (insert_position - begin(ids)), Component{std::forward<Args>(args)...});
				} else {
					inserted_component = components.emplace(begin(components) + (insert_position - begin(ids)), std::forward<Args>(args)...);
				}
				ids.insert(insert_position, id);
				add_remover<Component>();
				assert_all(std::is_sorted(begin(ids), end(ids)));
				return *inserted_component;
			}
			//add a component to an Entity
			template <class Component>
			Component &add(Component &&c) {
				return emplace<Component>(std::forward<Component>(c));
			}
			//get the component of a given type or nullptr if the Entity has no such component
			template <class Component>
			Component *get() {
				auto &ids = System::get_ids<Component>();
				auto id_it = lower_bound(begin(ids), end(ids), id);
				if (*id_it != id) {
					return nullptr;
				}
				auto pos = id_it - begin(ids);
				auto &components = System::get_components<Component>();
				return &components.at(pos);
			}
			//remove a component of a given type, UB if the entity has no such component, test with get to check if the entity has that component
			template <class Component>
			void remove() {
				auto &ids = System::get_ids<Component>();
				auto id_pos = lower_bound(begin(ids), end(ids), id);
				assert_fast(id_pos != end(ids));
				remove_remover<Component>(id_pos - begin(ids));
			}
			//check if the entity is valid. An entity becomes invalid when it is moved from
			bool is_valid() const {
				return id != max_id;
			}

			private:
			//remove a component of the given type and id
			template <class Component>
			static void remover(Impl::Id_t id) {
				auto &ids = System::get_ids<Component>();
				auto id_it = lower_bound(begin(ids), end(ids), id);
				assert_fast(*id_it == id); //make sure the component to remove exists
				auto &components = System::get_components<Component>();
				components.erase(begin(components) + (id_it - begin(ids)));
				ids.erase(id_it);
				assert_all(std::is_sorted(begin(ids), end(ids)));
			}

			template <class Component>
			void add_remover() {
				Remover r(id, remover<Component>, typeid(Component).name());
				auto pos = std::lower_bound(begin(removers), end(removers), r);
				removers.insert(pos, std::move(r));
				assert_all(std::is_sorted(begin(removers), end(removers)));
			}

			template <class Component>
			void remove_remover(int index) {
				removers.erase(begin(removers) + index);
			}

			//a struct to remove a component. This is unfortunately necessary, because entities don't know the types of their components
			struct Remover {
				Remover(Impl::Id_t id, void (*f)(Impl::Id_t), const char *type_name)
					: f(f)
					, id(id)
					, type_name(type_name) {
					Log::log_debug() << "++++++++Create remover for Entity " << id << " Component " << Utility::type_name(type_name);
				}
				Remover(Remover &&other) noexcept
					: f(other.f)
					, id(other.id)
					, type_name(other.type_name) {
					other.f = remover_dummy;
				}
				Remover &operator=(Remover &&other) noexcept {
					using std::swap;
					swap(id, other.id);
					swap(f, other.f);
					swap(type_name, other.type_name);
					return *this;
				}
				~Remover() {
					if (f != remover_dummy) {
						Log::log_debug() << "--------------------Remove Entity " << id << " Component " << Utility::type_name(type_name);
						f(id);
					}
				}
				bool operator<(const Remover &other) const {
					return std::tie(id, f) < std::tie(other.id, other.f);
				}
				bool operator<(Impl::Id_t other_id) const {
					return id < other_id;
				}
				bool operator>(Impl::Id_t other_id) const {
					return id > other_id;
				}

				private:
				//data
				void (*f)(Impl::Id_t);
				Impl::Id_t id;
				const char *type_name;
				//empty function to put into removers that have been moved from
				static inline void remover_dummy(Impl::Id_t /*unused*/) {}
			};
			//it is important that removers is cleared before the system component vectors are destroyed
			//it is also necessary to have removers be destroyed after all entities, because entities access removers in the destructor, don't know how to do
			//that without leaking removers
			protected:
			static Impl::Id_t id_counter;
			Impl::Id_t id;
			static std::vector<Remover> removers;
			friend bool operator<(Impl::Id_t id, const Remover &r);
		};
		inline bool operator<(Impl::Id_t id, const Entity_base::Remover &r) {
			return r > id;
		}
	} // namespace Impl
} // namespace ECS

#endif // ENTITY_BASE_H
