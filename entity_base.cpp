#include "entity_base.h"

ECS::Impl::Id_t ECS::Impl::Entity_base::id_counter;
std::vector<ECS::Impl::Entity_base::Remover> ECS::Impl::Entity_base::removers;
