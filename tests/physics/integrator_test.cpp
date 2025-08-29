#include "gtest/gtest.h"
#include <format>
#include <string>

#include "physics/Integrator.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <memory>
#include <vector>

using namespace APE::Physics;

constexpr float ERROR_BOUND = 0.75f;

struct Scenario {
	RigidBody start;
	std::vector<glm::vec3> expected_per_sec;
};

class IntegratorTest : public testing::Test {
protected:
	std::unique_ptr<Integrator> euler;

	std::vector<Scenario> scenarios;

	IntegratorTest()
	{
		euler = std::make_unique<EulerIntegrator>();

		fillScenarios();
	}

	void fillScenarios()
	{
		Scenario freefall {
			.start = {
				.pos = { 0, 100, 0 },
				.vel_linear = { 0, 0, 0 },
				.mass = 1.f,
				.forces = { 0, -9.8f, 0 },
			},
			.expected_per_sec = { 
				{ 0, 95.1, 0 },
				{ 0, 80.4, 0 },
				{ 0, 55.9, 0 },
				{ 0, 21.6, 0 },
				{ 0, -22.5, 0 },
			},
		};
		scenarios.emplace_back(std::move(freefall));
	}
};

bool isNear(const glm::vec3& a, const glm::vec3& b, float epsilon)
{
	return glm::length(a - b) < epsilon;
}

std::string printError(const glm::vec3& got, const glm::vec3& expected)
{
	return std::format(
		"Integrator should be closer to implicit solution:\n"
		"Got {}, Expected {}.",
		glm::to_string(got),
		glm::to_string(expected)
	);
}

TEST_F(IntegratorTest, Init)
{
	ASSERT_TRUE(euler)
		<< "Euler integrator should be instantiated.";
}

TEST_F(IntegratorTest, EulerFreefall)
{
	Scenario freefall = scenarios[0];

	auto fps_array = std::array { 1, 12, 30, 60, 144, 240, 500, 1000 };
	for (int fps : fps_array) {
		float dt = 1.f / fps;
		RigidBody curr = freefall.start;
		for (auto& expected_pos : freefall.expected_per_sec) {
			for (int i = 0; i < fps; ++i) {
				euler->integrate(curr, dt);
			}
			EXPECT_TRUE(isNear(curr.pos, expected_pos, ERROR_BOUND * dt))
				<< printError(curr.pos, expected_pos);
		}
	}
	
}





