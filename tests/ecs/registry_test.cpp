#include "gtest/gtest.h"

#include "ecs/Registry.h"

using namespace APE::ECS;

class RegistryTest : public testing::Test {
protected:
	Registry r;
	Registry r_filled;

	RegistryTest()
	{

	}
};


/*
 * Basic Initialization Tests
*/
TEST_F(RegistryTest, Init)
{
	ASSERT_EQ(r.numEntities(), 0) << "r should have no entities.";
	ASSERT_EQ(r_filled.numComponents(), 0) << "r should have no components.";
}



