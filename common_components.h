#ifndef COMMON_COMPONENTS_H
#define COMMON_COMPONENTS_H

#include <chrono>

namespace Common_components {
	struct Speed {
		float speed;
	};
	struct HP {
		HP(int hp)
			: hp(hp)
			, max_hp(hp) {}
		int hp;
		int max_hp;
	};
	struct Life_time {
		int life_time; //in logical frames left
	};
	//Tags
	struct Enemy {}; //set for all targetable enemies
	struct Map {};   //set for the map the player is playing on
	struct Run_straight_AI {};
} // namespace Common_components

#endif // COMMON_COMPONENTS_H
