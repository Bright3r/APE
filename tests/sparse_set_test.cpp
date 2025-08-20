#include <gtest/gtest.h>

#include "util/SparseSet.h"

#include <limits>

TEST(SparseSetTest, Init) 
{
	APE::SparseSet<size_t, int> set;

	ASSERT_EQ(set.tombstone(), std::numeric_limits<size_t>::max())
		<< "Tombstone must be the max of an integral type.";

	EXPECT_TRUE(set.empty()) << "Newly construct set should be empty.";
	EXPECT_EQ(set.getSize(), 0) << "Set size should be 0 when empty.";
}
