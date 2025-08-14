cmake_minimum_required(VERSION 3.20)

set (CMAKE_CXX_STANDARD 20)
set(CMAKE_TOOLCHAIN_FILE "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

project(ape)

find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_executable(ape 
	src/main.cpp
	src/App.cpp
	src/core/AppRunner.cpp
	src/render/Shader.cpp
	src/render/Context.cpp
	src/render/Renderer.cpp
	src/render/Model.cpp
	src/render/Image.cpp
)

target_compile_options(ape PRIVATE
    -fno-omit-frame-pointer
    -fsanitize=address
)
target_link_options(ape PRIVATE -fsanitize=address)



target_include_directories(ape PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(ape PRIVATE ${Stb_INCLUDE_DIR})

target_link_libraries(ape PRIVATE 
	SDL3::SDL3
	glm::glm
	spdlog::spdlog
	assimp::assimp
	imgui::imgui
)
