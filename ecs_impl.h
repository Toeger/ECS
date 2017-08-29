#ifndef ECS_IMPL_H
#define ECS_IMPL_H

#include <limits>

namespace ECS {
	//every Entity gets a unique Id
	namespace Impl {
		using Id_t = long long unsigned int;
		constexpr Id_t max_id = std::numeric_limits<Id_t>::max();
	} // namespace Impl
} // namespace ECS

#endif // ECS_IMPL_H
