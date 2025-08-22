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
find_package(GTest CONFIG REQUIRED)

# Compile Engine into lib
add_library(
	ape_lib
	src/App.cpp
	src/core/AppRunner.cpp
	src/render/Shader.cpp
	src/render/Context.cpp
	src/render/Renderer.cpp
	src/render/Model.cpp
	src/render/Image.cpp
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
	tests/pool_test.cpp
)

target_link_libraries(
	tests PRIVATE
	ape_lib
	GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)

