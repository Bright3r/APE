#include "gtest/gtest.h"

#include "ecs/Registry.h"

#include <glm/glm.hpp>
#include <string>
#include <unordered_set>
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

	bool operator==(const PhysComp& other) const
	{
		return pos == other.pos &&
			vel == other.vel &&
			accel == other.accel;
	}
};

struct NameComp {
	std::string first_name;
	std::string last_name;

	bool operator==(const NameComp& other) const
	{
		return first_name == other.first_name &&
			last_name == other.last_name;
	}
};


/*
 * Test Fixture
*/
class RegistryTest : public testing::Test {
protected:
	Registry r;

	Registry r_filled;
	std::vector<Registry::EntityHandle> ents_filled;

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

	for (const auto ent : ents_filled) {
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

	auto ent = r.createEntity();
	EXPECT_EQ(r.numEntities(), 1)
		<< "r should have 1 entity.";

	EXPECT_EQ(r_filled.numEntities(), ents_filled.size())
		<< "r_filled should have " << ents_filled.size() << " entities.";
}

/*
 * Removing Entities
*/
TEST_F(RegistryTest, BasicRemoveEntity)
{
	size_t len { 10 };
	std::vector<Registry::EntityHandle> ents;
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
	auto ent = r.createEntity();
	r.emplaceComponent<PosComp>(ent, -1, -2, -3);

	EXPECT_TRUE(r.hasComponent<PosComp>(ent))
		<< "Entity " << ent.id << " should have Position Component.";

	r.clearComponent<PosComp>();
	EXPECT_FALSE(r.hasComponent<PosComp>(ent))
		<< "Entity " << ent.id << " should not have Position Component.";
}

TEST_F(RegistryTest, ComponentLifecycle)
{
	auto ent = r.createEntity();
	auto ent2 = r.createEntity();
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
	auto ent = r.createEntity();
	r.emplaceComponent<NameComp>(ent, "Hello", "World");

	auto comp = r.getComponent<NameComp>(ent);
	NameComp expected { "Hello", "World" };
	EXPECT_EQ(comp, expected)
		<< "Name Component should be Hello World.";
}

TEST_F(RegistryTest, GetMissingComponent)
{
	auto ent = r.createEntity();
	EXPECT_DEATH({
		auto comp = r.getComponent<NameComp>(ent);
	}, "");
}

TEST_F(RegistryTest, BasicGetComponents)
{
	auto ent = r.createEntity();
	PosComp pos_comp { 1, 2, 3 };
	PhysComp phys_comp {
		glm::vec3(0, 0, 0),
		glm::vec3(1, 1, 1),
		glm::vec3(-1, -1, -1)
	};
	r.emplaceComponent<PosComp>(ent, pos_comp);
	r.emplaceComponent<PhysComp>(ent, phys_comp);

	auto comps = r.getComponents<PosComp, PhysComp>(ent);
	EXPECT_EQ(std::get<0>(comps), pos_comp)
		<< "Position Components should be equal.";
	EXPECT_EQ(std::get<1>(comps), phys_comp)
		<< "Physics Components should be equal.";

	auto comps2 = r.getComponents<PhysComp, PosComp>(ent);
	EXPECT_EQ(std::get<0>(comps2), phys_comp)
		<< "Physics Components should be equal.";
	EXPECT_EQ(std::get<1>(comps2), pos_comp)
		<< "Position Components should be equal.";
}

TEST_F(RegistryTest, GetSameComponents)
{
	auto ent = r.createEntity();
	PosComp pos_comp { 1, 2, 3 };
	PhysComp phys_comp {
		glm::vec3(0, 0, 0),
		glm::vec3(1, 1, 1),
		glm::vec3(-1, -1, -1)
	};
	r.emplaceComponent<PosComp>(ent, pos_comp);
	r.emplaceComponent<PhysComp>(ent, phys_comp);

	auto comps = r.getComponents<PosComp, PosComp>(ent);
	EXPECT_EQ(std::get<0>(comps), pos_comp)
		<< "Position Components should be equal.";
}

TEST_F(RegistryTest, GetMissingComponents)
{
	auto ent = r.createEntity();
	EXPECT_DEATH({
		auto comp = (r.getComponents<PosComp, PhysComp>(ent));
	}, "");
}


/*
 * Checking Components
*/
TEST_F(RegistryTest, BasicHasComponent)
{
	auto ent = r.createEntity();
	r.emplaceComponent<PosComp>(ent, 1, 2, 3);

	EXPECT_TRUE(r.hasComponent<PosComp>(ent))
		<< "Entity should have Position Component.";
	EXPECT_FALSE(r.hasComponent<PhysComp>(ent))
		<< "Entity should not have Physics Component.";
	EXPECT_FALSE(r.hasComponent<NameComp>(ent))
		<< "Entity should not have Name Component.";

	r.emplaceComponent<NameComp>(ent, "First", "Last");
	EXPECT_TRUE(r.hasComponent<NameComp>(ent))
		<< "Entity should have Name Component.";
}

TEST_F(RegistryTest, BasicHasAllComponents)
{	
	auto ent = r.createEntity();
	r.emplaceComponent<PosComp>(ent, 1, 2, 3);

	EXPECT_TRUE(r.hasAllComponents<PosComp>(ent))
		<< "Entity should have Position Component.";
	EXPECT_FALSE((r.hasAllComponents<PosComp, PhysComp, NameComp>(ent)))
		<< "Entity should not have all components.";

	r.emplaceComponent<NameComp>(ent, "First", "Last");
	EXPECT_TRUE((r.hasAllComponents<PosComp, NameComp>(ent)))
		<< "Entity should have Position and Name Components.";
	EXPECT_FALSE((r.hasAllComponents<PosComp, PhysComp, NameComp>(ent)))
		<< "Entity should not have all components.";

	r.emplaceComponent<PhysComp>(ent);
	EXPECT_TRUE((r.hasAllComponents<PosComp, PhysComp, NameComp>(ent)))
		<< "Entity should have all components.";
}

TEST_F(RegistryTest, BasicHasAnyComponent)
{
	auto ent = r.createEntity();
	EXPECT_FALSE((r.hasAnyComponent<PosComp, PhysComp, NameComp>(ent)))
		<< "Entity should not have any components.";

	r.emplaceComponent<PosComp>(ent, 1, 2, 3);
	EXPECT_TRUE((r.hasAnyComponent<PosComp, PhysComp, NameComp>(ent)))
		<< "Entity should have Position Component.";
}


/*
 * Getting Views
*/
TEST_F(RegistryTest, BasicView)
{
	auto e1 = r.createEntity();
	auto e2 = r.createEntity();
	auto e3 = r.createEntity();

	r.emplaceComponent<PosComp>(e1, 1, 2, 3);
	r.emplaceComponent<PosComp>(e2, 4, 5, 6);
	r.emplaceComponent<PosComp>(e3, 7, 8, 9);

	auto ent_view = r.view<PosComp>();
	EXPECT_EQ(ent_view.size(), 3)
		<< "EntityView should have 3 entities.";

	std::unordered_set<EntityID> seen;
	for (auto& e : ent_view) {
		seen.insert(e.id);
	}

	EXPECT_TRUE(seen.erase(e1.id))
		<< "Entity e1 should be in the EntityView.";
	EXPECT_TRUE(seen.erase(e2.id))
		<< "Entity e2 should be in the EntityView.";
	EXPECT_TRUE(seen.erase(e3.id))
		<< "Entity e3 should be in the EntityView.";
}

TEST_F(RegistryTest, EmptyView)
{
	auto ent_view = r.view<PosComp, NameComp>();
	EXPECT_TRUE(ent_view.empty())
		<< "EntityView should be empty.";
}

TEST_F(RegistryTest, RemoveEntityFromView)
{
	auto e1 = r.createEntity();
	auto e2 = r.createEntity();
	r.emplaceComponent<PosComp>(e1);
	r.emplaceComponent<PosComp>(e2);

	auto ent_view = r.view<PosComp>();
	EXPECT_EQ(ent_view.size(), 2)
		<< "EntityView should be have 2 entities.";

	for (auto e : ent_view) {
		r.destroyEntity(e);
	}

	EXPECT_EQ(r.numEntities(), 0)
		<< "r should have no remaining entities.";
}

TEST_F(RegistryTest, RemoveComponentFromView)
{
	auto e1 = r.createEntity();
	auto e2 = r.createEntity();
	r.emplaceComponent<PosComp>(e1);
	r.emplaceComponent<PosComp>(e2);

	auto ent_view = r.view<PosComp>();
	EXPECT_EQ(ent_view.size(), 2)
		<< "EntityView should be have 2 entities.";

	for (auto e : ent_view) {
		r.removeComponent<PosComp>(e);
	}

	EXPECT_FALSE(r.hasComponent<PosComp>(e1))
		<< "Entity e1 should not have Position Component.";
	EXPECT_FALSE(r.hasComponent<PosComp>(e2))
		<< "Entity e2 should not have Position Component.";

	ent_view = r.view<PosComp>();
	EXPECT_TRUE(ent_view.empty())
		<< "EntityView should be empty.";
}

TEST_F(RegistryTest, ModifyComponentFromView)
{
	auto e1 = r.createEntity();
	auto e2 = r.createEntity();
	r.emplaceComponent<PosComp>(e1);
	r.emplaceComponent<PosComp>(e2);

	auto ent_view = r.view<PosComp>();
	for (auto e : ent_view) {
		auto& pos = r.getComponent<PosComp>(e);
		pos.x = 5;
	}

	EXPECT_EQ(r.getComponent<PosComp>(e1).x, 5)
		<< "Entity e1 should have Position x-coord of 5.";
	EXPECT_EQ(r.getComponent<PosComp>(e2).x, 5)
		<< "Entity e1 should have Position x-coord of 5.";
}

TEST_F(RegistryTest, MultiComponentView)
{
	auto e1 = r.createEntity();
	auto e2 = r.createEntity();
	r.emplaceComponent<PosComp>(e1);
	r.emplaceComponent<PosComp>(e2);

	auto view = r.view<PosComp>();
	ASSERT_EQ(view.size(), 2)
		<< "View should have 2 entities.";

	r.emplaceComponent<PhysComp>(e1);
	view = r.view<PosComp, PhysComp>();
	ASSERT_EQ(view.size(), 1)
		<< "View should have 1 entity.";

	view = r.view<PosComp, NameComp>();
	ASSERT_TRUE(view.empty())
		<< "View should be empty.";

	r.emplaceComponent<NameComp>(e2);
	view = r.view<PosComp, NameComp>();
	ASSERT_EQ(view.size(), 1)
		<< "View should have 1 entity.";

	view = r.view<PosComp, PhysComp, NameComp>();
	ASSERT_TRUE(view.empty())
		<< "View should be empty.";

	r.removeComponent<PhysComp>(e1);
	view = r.view<PhysComp>();
	ASSERT_TRUE(view.empty())
		<< "View should be empty.";

	r.removeComponent<PosComp>(e1);
	view = r.view<PosComp>();
	ASSERT_EQ(view.size(), 1)
		<< "View should have 1 entity.";
}

