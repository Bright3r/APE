#include "gtest/gtest.h"

#include "ecs/Registry.h"
#include "util/Logger.h"

#include <glm/glm.hpp>
#include <string>
#include <utility>


using namespace APE::ECS;

/*
 * Dummy Components
*/
struct PosComp {
	float x, y, z;

	bool operator==(const PosComp& other) const
	{
		return x == other.x &&
			y == other.y &&
			z == other.z;
	}
};

struct PhysComp {
	glm::vec3 pos;
	glm::vec3 vel;
	glm::vec3 accel;
};

struct NameComp {
	std::string first_name;
	std::string last_name;
};


/*
 * Test Fixture
*/
class RegistryTest : public testing::Test {
protected:
	Registry r;

	Registry r_filled;
	std::vector<Entity> ents_filled;

	RegistryTest()
	{
		auto ent = r_filled.createEntity();
		r_filled.emplaceComponent<PosComp>(ent, 1, 2, 3);
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


/*
 * Adding Entities
*/
TEST_F(RegistryTest, BasicCreateEntity)
{
	auto ent = r.createEntity();
	EXPECT_EQ(r.numEntities(), 1) << "r should have 1 entity.";

	ent = r.createEntity();
	EXPECT_EQ(r.numEntities(), 2) << "r should have 2 entities.";

	for (int i = 3; i <= 10; ++i) {
		ent = r.createEntity();
	}
	EXPECT_EQ(r.numEntities(), 10) << "r should have 10 entities.";
}


/*
* Checking Entities
*/
TEST_F(RegistryTest, BasicNumEntities)
{
	EXPECT_EQ(r.numEntities(), 0)
		<< "r should have 0 entities.";

	EXPECT_EQ(r_filled.numEntities(), ents_filled.size())
		<< "r_filled should have " << ents_filled.size() << " entities.";
}

/*
 * Removing Entities
*/
TEST_F(RegistryTest, BasicRemoveEntity)
{
	size_t len { 10 };
	std::vector<Entity> ents;
	for (int i = 0; i < len; ++i) {
		ents.emplace_back(r.createEntity());
	}
	EXPECT_EQ(r.numEntities(), len) << "r should have 10 entities.";

	for (int i = 0; i < len; ++i) {
		EXPECT_TRUE(r.destroyEntity(ents.back())) 
			<< "r's last entity should be destroyed.";
		ents.pop_back();

		EXPECT_EQ(r.numEntities(), ents.size())
			<< "r should have " << ents.size() << " entities.";
	}
}


/*
 * Adding Components
*/
TEST_F(RegistryTest, BasicEmplaceComponent)
{
	auto ent = r.createEntity();
	r.emplaceComponent<PosComp>(ent, -1, -2, -3);

	EXPECT_TRUE(r.hasComponent<PosComp>(ent))
		<< "Entity should have Position Component.";

	bool has_any = r.hasAnyComponent<PhysComp, NameComp>(ent);
	EXPECT_FALSE(has_any)
		<< "Entity should not have Physics or Name Components.";
}

TEST_F(RegistryTest, EmplaceComponentFromCopy)
{
	auto ent = r.createEntity();
	PhysComp comp(glm::vec3(1, 2, 3), glm::vec3(4, 5, 6), glm::vec3(7, 8, 9));
	r.emplaceComponent<PhysComp>(ent, comp);

	EXPECT_TRUE(r.hasComponent<PhysComp>(ent))
		<< "Entity should have Physics Component.";

	bool has_any = r.hasAnyComponent<PosComp, NameComp>(ent);
	EXPECT_FALSE(has_any)
		<< "Entity should not have Position or Name Components.";
}

TEST_F(RegistryTest, EmplaceComponentFromMove)
{
	auto ent = r.createEntity();
	PhysComp comp(glm::vec3(1, 2, 3), glm::vec3(4, 5, 6), glm::vec3(7, 8, 9));
	r.emplaceComponent<PhysComp>(ent, std::move(comp));

	EXPECT_TRUE(r.hasComponent<PhysComp>(ent))
		<< "Entity should have Physics Component.";

	bool has_any = r.hasAnyComponent<PosComp, NameComp>(ent);
	EXPECT_FALSE(has_any)
		<< "Entity should not have Position or Name Components.";
}

TEST_F(RegistryTest, BasicReplaceComponent)
{
	auto ent = r.createEntity();
	r.emplaceComponent<PosComp>(ent, -1, -2, -3);

	PosComp comp { 3, 5, 7 };
	r.replaceComponent<PosComp>(ent, comp);

	EXPECT_TRUE(r.hasComponent<PosComp>(ent))
		<< "Entity should have Position Component.";

	bool has_any = r.hasAnyComponent<PhysComp, NameComp>(ent);
	EXPECT_FALSE(has_any)
		<< "Entity should not have Physics or Name Components.";

	auto pos = r.getComponent<PosComp>(ent);
	EXPECT_TRUE(pos == comp)
		<< "Entity's Position Component should be equal.";
}

TEST_F(RegistryTest, ReplaceMissingComponent)
{
	auto ent = r.createEntity();
	EXPECT_DEATH({
		r.replaceComponent<PosComp>(ent, 3, 5, 7);
	}, "");
}

TEST_F(RegistryTest, ReplaceComponentFromCopy)
{
	auto ent = r.createEntity();
	r.emplaceComponent<PosComp>(ent, -1, -2, -3);

	PosComp comp { 3, 5, 7 };
	r.replaceComponent<PosComp>(ent, comp);

	EXPECT_TRUE(r.hasComponent<PosComp>(ent))
		<< "Entity should have Position Component.";

	bool has_any = r.hasAnyComponent<PhysComp, NameComp>(ent);
	EXPECT_FALSE(has_any)
		<< "Entity should not have Physics or Name Components.";

	auto pos = r.getComponent<PosComp>(ent);
	EXPECT_TRUE(pos == comp)
		<< "Entity's Position Component should be equal.";
}

TEST_F(RegistryTest, ReplaceComponentFromMove)
{
	auto ent = r.createEntity();
	r.emplaceComponent<PosComp>(ent, -1, -2, -3);

	PosComp comp { 3, 5, 7 };
	PosComp comp_copy = comp;
	r.replaceComponent<PosComp>(ent, std::move(comp));

	EXPECT_TRUE(r.hasComponent<PosComp>(ent))
		<< "Entity should have Position Component.";

	bool has_any = r.hasAnyComponent<PhysComp, NameComp>(ent);
	EXPECT_FALSE(has_any)
		<< "Entity should not have Physics or Name Components.";

	auto pos = r.getComponent<PosComp>(ent);
	EXPECT_TRUE(pos == comp_copy)
		<< "Entity's Position Component should be equal.";
}

TEST_F(RegistryTest, BasicEmplaceOrReplaceComponent)
{
	auto ent = r.createEntity();
	r.emplaceOrReplaceComponent<PosComp>(ent, -1, -2, -3);

	PosComp comp { 3, 5, 7 };
	r.emplaceOrReplaceComponent<PosComp>(ent, comp);

	EXPECT_TRUE(r.hasComponent<PosComp>(ent))
		<< "Entity should have Position Component.";

	bool has_any = r.hasAnyComponent<PhysComp, NameComp>(ent);
	EXPECT_FALSE(has_any)
		<< "Entity should not have Physics or Name Components.";

	auto pos = r.getComponent<PosComp>(ent);
	EXPECT_TRUE(pos == comp)
		<< "Entity's Position Component should be equal.";
}


/*
 * Removing Components
*/
TEST_F(RegistryTest, BasicRemoveComponent)
{
	auto ent = r.createEntity();
	r.emplaceComponent<PosComp>(ent, -1, -2, -3);

	EXPECT_TRUE(r.hasComponent<PosComp>(ent))
		<< "Entity should have Position Component.";

	r.removeComponent<PosComp>(ent);
	EXPECT_FALSE(r.hasComponent<PosComp>(ent))
		<< "Entity should not have Position Component.";
}

TEST_F(RegistryTest, BasicClearComponent)
{
	auto& ent = r.createEntity();
	r.emplaceComponent<PosComp>(ent, -1, -2, -3);

	EXPECT_TRUE(r.hasComponent<PosComp>(ent))
		<< "Entity " << ent.id << " should have Position Component.";

	APE_TRACE("Should clear entity {} of component #{}.",
		ent.id,
		Registry::typeID<PosComp>()
	);

	r.clearComponent<PosComp>();
	EXPECT_FALSE(r.hasComponent<PosComp>(ent))
		<< "Entity " << ent.id << " should not have Position Component.";
}

TEST_F(RegistryTest, ComponentLifecycle)
{
	auto& ent = r.createEntity();
	auto& ent2 = r.createEntity();
	r.emplaceOrReplaceComponent<PosComp>(ent, -1, -2, -3);

	PosComp comp { 3, 5, 7 };
	r.emplaceOrReplaceComponent<PosComp>(ent, comp);
	r.emplaceComponent<PosComp>(ent2, comp);

	EXPECT_TRUE(r.hasComponent<PosComp>(ent))
		<< "Entity should have Position Component.";
	EXPECT_TRUE(r.hasComponent<PosComp>(ent2))
		<< "Entity should have Position Component.";

	auto pos = r.getComponent<PosComp>(ent);
	EXPECT_TRUE(pos == comp)
		<< "Entity's Position Component should be equal.";

	r.removeComponent<PosComp>(ent);
	EXPECT_FALSE(r.hasComponent<PosComp>(ent))
		<< "Entity should not have Position Component.";
	EXPECT_TRUE(r.hasComponent<PosComp>(ent2))
		<< "Entity should have Position Component.";

	r.clearComponent<PosComp>();
	EXPECT_FALSE(r.hasComponent<PosComp>(ent))
		<< "Entity should not have Position Component.";
	EXPECT_FALSE(r.hasComponent<PosComp>(ent2))
		<< "Entity should not have Position Component.";

	r.emplaceComponent<PosComp>(ent, 1, 2, 3);
	EXPECT_TRUE(r.hasComponent<PosComp>(ent))
		<< "Entity should have Position Component.";
	EXPECT_FALSE(r.hasComponent<PosComp>(ent2))
		<< "Entity should not have Position Component.";

	r.emplaceComponent<NameComp>(ent2, "Hello", "World");
	EXPECT_FALSE(r.hasComponent<NameComp>(ent))
		<< "Entity should not have Name Component.";
	EXPECT_TRUE(r.hasComponent<NameComp>(ent2))
		<< "Entity should have Name Component.";
}


/*
* Getting Components
*/
TEST_F(RegistryTest, BasicGetComponent)
{

}

TEST_F(RegistryTest, GetMissingComponent)
{

}

TEST_F(RegistryTest, BasicGetComponents)
{

}


/*
 * Checking Components
*/
TEST_F(RegistryTest, BasicHasComponent)
{

}

TEST_F(RegistryTest, BasicHasAllComponents)
{

}

TEST_F(RegistryTest, BasicHasAnyComponent)
{

}

TEST_F(RegistryTest, BasicNumComponents)
{

}


/*
 * Getting Views
*/
TEST_F(RegistryTest, BasicEntityView)
{

}

TEST_F(RegistryTest, RemoveEntityFromView)
{

}

TEST_F(RegistryTest, RemoveComponentFromView)
{

}






