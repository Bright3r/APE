#include "gtest/gtest.h"
#include <string>

#include "ecs/Registry.h"

using namespace APE::ECS;


/*
 * Dummy Components
*/
struct PosComp {
	int x, y, z;
};

struct PhysComp {
	float x, y, z;
	std::array<float, 3> vel;
	std::array<float, 3> accel;
};

struct NameComp {
	std::string first_name;
	std::string last_name;
};


class RegistryTest : public testing::Test {
protected:
	Registry r;

	Registry r_filled;
	std::vector<Entity> ents_filled;

	RegistryTest()
	{
		auto ent = r_filled.createEntity();
		r_filled.emplaceComponent<PosComp>(ent);
		ents_filled.emplace_back(ent);
	}
};


/*
 * Basic Initialization Tests
*/
TEST_F(RegistryTest, Init)
{
	ASSERT_EQ(r.numEntities(), 0) << "r should have no entities.";
	ASSERT_EQ(r_filled.numEntities(), ents_filled.size()) 
		<< "r_filled's initial entities should be stored";

	for (const Entity& ent : ents_filled) {
		ASSERT_TRUE(r_filled.isValid(ent)) 
			<< "r_filled's initial entities should be valid.";
	}
}

TEST_F(RegistryTest, InitComponents)
{
	auto ent = ents_filled[0];

	bool has_pos = r_filled.hasComponent<PosComp>(ent);
	ASSERT_TRUE(has_pos) << "First entity should have Position Component.";

	bool has_any = r_filled.hasAnyComponent<PosComp, PhysComp, NameComp>(ent);
	ASSERT_TRUE(has_any) << "First entity should have any component.";

	bool has_all = r_filled.hasAllComponents<PosComp, PhysComp, NameComp>(ent);
	ASSERT_FALSE(has_all) << "First entity should not have all components.";
}



