#include "system.h"

std::vector<void (*)()> ECS::System::function_pointer_systems;
std::vector<std::function<void()>> ECS::System::function_systems;
