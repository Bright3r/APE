#pragma once

#include "physics/BVH.h"

namespace APE::Physics {

struct State {
	Collider::BVH bvh;
};

};
