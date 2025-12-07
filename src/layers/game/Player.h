#pragma once

#include "core/ecs/Registry.h"
#include "phys/PlayerController.h"

#include <memory>
#include <utility>

namespace APE
{

struct Player
{
	std::unique_ptr<Phys::PlayerController> controller;
	ECS::EntityHandle ent;

	Player(
		std::unique_ptr<Phys::PlayerController> controller,
		ECS::EntityHandle ent
	) noexcept
		: controller(std::move(controller))
		, ent(ent)
	{

	}
};

};	// end of namespace

