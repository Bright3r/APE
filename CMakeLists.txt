cmake_minimum_required(VERSION 3.20)

set (CMAKE_CXX_STANDARD 20)
set(CMAKE_TOOLCHAIN_FILE 
	"$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" CACHE STRING ""
)

project(ape)

# External Libraries
find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(cereal CONFIG REQUIRED)
find_package(nfd CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# Compile Engine into lib
add_library(
	ape_lib
	src/core/Engine.cpp
	src/core/input/Input.cpp
	src/core/render/Shader.cpp
	src/core/render/Context.cpp
	src/core/render/Renderer.cpp
	src/core/render/Image.cpp
	src/core/scene/ModelLoader.cpp
	src/core/scene/ImageLoader.cpp
	src/physics/Collisions.cpp
	src/layers/game/GameLayer.cpp
	src/layers/editor/EditorLayer.cpp
)

target_include_directories(
	ape_lib PUBLIC
	${CMAKE_SOURCE_DIR}/src
	${Stb_INCLUDE_DIR}
)

target_link_libraries(
	ape_lib PUBLIC
	SDL3::SDL3
	glm::glm
	spdlog::spdlog
	assimp::assimp
	imgui::imgui
	imguizmo::imguizmo
	cereal::cereal
	nfd::nfd
)


# Program Executable
add_executable(
	ape 
	src/main.cpp
)
target_link_libraries(
	ape PRIVATE 
	ape_lib
)

target_compile_options(
	ape PRIVATE
	-fno-omit-frame-pointer
	-fsanitize=address
)
target_link_options(
	ape PRIVATE
	-fsanitize=address
)


# Testing
enable_testing()

add_executable(
	tests
	tests/ecs/pool_test.cpp
	tests/ecs/registry_test.cpp
	tests/physics/integrator_test.cpp
)

target_link_libraries(
	tests PRIVATE
	ape_lib
	GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)

