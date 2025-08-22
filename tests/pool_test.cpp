#include "gtest/gtest.h"

#include "ecs/Pool.h"

#include <limits>

using namespace APE;

class PoolTest : public testing::Test {
protected:
	Pool<size_t, int> set;
	Pool<size_t, int> filled_set;

	PoolTest()
	{
		for (size_t i = 0; i < 50; ++i) {
			filled_set.emplace(i, 5*i);
		}
	}
};


/*
 * Basic Initialization Tests
*/
TEST_F(PoolTest, Init)
{
	EXPECT_TRUE(!filled_set.empty()) << "Set should not be empty.";
	EXPECT_EQ(filled_set.size(), 50) << "Set should be size 50.";
}

TEST_F(PoolTest, ValidTombstone) 
{
	ASSERT_EQ(set.tombstone(), std::numeric_limits<size_t>::max())
		<< "Tombstone must be the max of an integral type.";
}

TEST_F(PoolTest, EmptyInitially)
{
	EXPECT_TRUE(set.empty()) << "Newly construct set should be empty.";
	EXPECT_EQ(set.size(), 0) << "Set size should be 0 when empty.";
}


/*
 * Component Addition Tests
*/
TEST_F(PoolTest, BasicEmplace)
{
	set.emplace(0, 27);
	EXPECT_TRUE(!set.empty()) << "Set should not be empty.";
	EXPECT_EQ(set.size(), 1) << "Set should be size 1.";
}

TEST_F(PoolTest, BasicTryEmplace)
{
	set.tryEmplace(0, 27);
	EXPECT_TRUE(!set.empty()) << "Set should not be empty.";
	EXPECT_EQ(set.size(), 1) << "Set should be size 1.";
}

TEST_F(PoolTest, MixedEmplacement)
{
	set.emplace(0, 27);
	set.tryEmplace(0, 30);

	EXPECT_EQ(set.size(), 1) << "Set should be size 1.";
	EXPECT_EQ(set.get(0), 30) << "Set should have (0, 30).";

	EXPECT_DEATH({
		set.emplace(0, 27);
	}, "");
}

TEST_F(PoolTest, EmplaceCopy)
{
	int x = 27;
	set.emplace(0, x);
	ASSERT_EQ(set.size(), 1) << "Set should be size 1.";
	ASSERT_EQ(set.get(0), x) << "Set should have (0, 27)";
}

TEST_F(PoolTest, TryEmplaceCopy)
{
	int x = 27;
	set.tryEmplace(0, x);
	ASSERT_EQ(set.size(), 1) << "Set should be size 1.";
	ASSERT_EQ(set.get(0), x) << "Set should have (0, 27)";

	x = 30;
	set.tryEmplace(0, x);
	ASSERT_EQ(set.size(), 1) << "Set should be size 1.";
	ASSERT_EQ(set.get(0), x) << "Set should have (0, 27)";
}

/*
* Component Removal Tests
*/
TEST_F(PoolTest, RemoveFromEmpty)
{
	EXPECT_FALSE(set.remove(0)) << "Set cannot remove entity 0.";
}

TEST_F(PoolTest, RemoveFromBack)
{
	set.emplace(0, 27);
	EXPECT_TRUE(set.remove(0)) << "Set should remove entity 0.";

	EXPECT_FALSE(set.remove(0)) << "Set cannot remove entity 0.";
	EXPECT_FALSE(set.contains(0)) << "Set should no longer contain entity 0.";

	set.emplace(1, 42);
	EXPECT_FALSE(set.remove(0)) << "Set cannot remove entity 0.";
	EXPECT_FALSE(set.contains(0)) << "Set should no longer contain entity 0.";
}

TEST_F(PoolTest, RemoveFromMiddle)
{
	EXPECT_TRUE(filled_set.remove(25)) << "Set should have entity 25 removed.";
	EXPECT_EQ(filled_set.size(), 49) << "Set should be size 49.";
	EXPECT_FALSE(filled_set.remove(25)) << "Set should no longer have entity 25.";

	for (size_t i = 0; i < 50; ++i) {
		if (i == 25) {
			continue;
		}
		EXPECT_TRUE(filled_set.contains(i)) 
			<< "Set should contain entity " << i;
	}

	EXPECT_DEATH({
		auto x = filled_set.get(25);
	}, "");
}

TEST_F(PoolTest, RemoveFromFront)
{
	for (size_t i = 0; i < 50; ++i) {
		ASSERT_TRUE(filled_set.remove(i)) << "Set should remove entity " << i;
	}
	ASSERT_TRUE(filled_set.empty()) << "Set should be empty.";

	for (size_t i = 0; i < 50; ++i) {
		ASSERT_TRUE(!filled_set.contains(i)) 
			<< "Set should not contain entity " << i;
	}

	filled_set.emplace(25, 100);
	ASSERT_EQ(filled_set.get(25), 100) << "Set should have (25, 100).";

	ASSERT_TRUE(filled_set.remove(25)) << "Set should remove entity 25.";
	EXPECT_DEATH({
		auto x = filled_set.get(25);
	}, "");
}

TEST_F(PoolTest, BasicClear)
{
	filled_set.clear();
	EXPECT_TRUE(filled_set.empty()) << "Set should be empty.";
	EXPECT_EQ(filled_set.size(), 0) << "Set should be size 0.";

	for (size_t i = 0; i < 50; ++i) {
		EXPECT_TRUE(!filled_set.contains(i)) 
			<< "filled_set should not contain entity " << i;
	}
}


/*
* Component Retrieval Tests
*/

TEST_F(PoolTest, BasicContains)
{
	EXPECT_TRUE(!set.contains(0)) << "Set should not have entity 0.";
	EXPECT_TRUE(!set.contains(100)) << "Set should not have entity 100.";

	set.emplace(0, 27);
	EXPECT_TRUE(set.contains(0)) << "Set should contain entity 0.";
	EXPECT_TRUE(!set.contains(100)) << "Set should not have entity 100.";

	set.emplace(100);
	EXPECT_TRUE(set.contains(0)) << "Set should contain entity 0.";
	EXPECT_TRUE(set.contains(100)) << "Set should contain entity 100.";

	EXPECT_TRUE(!set.contains(42)) << "Set should not have entity 42.";
	set.emplace(42, 0);
	EXPECT_TRUE(set.contains(42)) << "Set should contain entity 42.";
}


TEST_F(PoolTest, BasicGet)
{
	set.emplace(0, 27);
	EXPECT_EQ(set.get(0), 27) << "Set should have (0, 27).";

	set.set(0, 30);
	EXPECT_EQ(set.get(0), 30) << "Set should have (0, 30).";
	EXPECT_EQ(set.size(), 1) << "Set should be size 1.";
}

TEST_F(PoolTest, GetRemovedComponent)
{
	set.emplace(0, 27);
	EXPECT_EQ(set.get(0), 27) << "Set should have (0, 27).";

	EXPECT_TRUE(set.remove(0)) << "Set should remove entity 0.";
	EXPECT_TRUE(set.empty()) << "Set should be empty.";
	
	EXPECT_DEATH({
		auto x = set.get(0);
	}, "");
}

TEST_F(PoolTest, BasicSet)
{
	filled_set.set(0, 100);
	EXPECT_EQ(filled_set.get(0), 100) << "Set should have (0, 100)";

	filled_set.set(25, 100);
	EXPECT_EQ(filled_set.get(25), 100) << "Set should have (25, 100)";

	filled_set.set(49, 100);
	EXPECT_EQ(filled_set.get(49), 100) << "Set should have (49, 100)";

	EXPECT_DEATH({
		filled_set.set(50, 100);
	}, "");
}

TEST_F(PoolTest, BasicIteration)
{
	size_t expected_id = 49;
	int expected_val = 5*expected_id;
	for (auto [ id, val ] : filled_set) {
		EXPECT_EQ(id, expected_id);
		EXPECT_EQ(val, expected_val);

		expected_id--;
		expected_val = 5*expected_id;
	}
}

TEST_F(PoolTest, RemoveWhileIterating)
{
	for (auto [ id, val ] : filled_set) {
		EXPECT_TRUE(filled_set.remove(id)) 
			<< "Set should remove entity " << id;
	}

	ASSERT_TRUE(filled_set.empty()) << "Set should be empty.";
}





