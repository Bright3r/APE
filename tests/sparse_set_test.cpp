#include <catch2/catch_test_macros.hpp>

#include "util/SparseSet.h"

#include <limits>

TEST_CASE("SparseSet Instantiated", "[SparseSet]") 
{
	APE::SparseSet<size_t, int> set;

	SECTION("SparseSet<size_t, int> initialization") 
	{
		REQUIRE(set.tombstone() == std::numeric_limits<size_t>::max());
		REQUIRE(set.empty());
		REQUIRE(set.getSize() == 0);
	}
}
