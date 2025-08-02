cmake_minimum_required(VERSION 3.20)

set(CMAKE_TOOLCHAIN_FILE "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

project(ape)

find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_executable(ape 
	src/main.cpp
	src/App.cpp
	src/AppRunner.cpp
	src/render/Shader.cpp
	src/render/Context.cpp
	src/render/Renderer.cpp
	src/render/Mesh.cpp
)

target_compile_options(ape PRIVATE
    -fsanitize=address
    -fno-omit-frame-pointer
)
target_link_options(ape PRIVATE -fsanitize=address)



target_include_directories(ape PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(ape PRIVATE 
	SDL3::SDL3
	glm::glm
	spdlog::spdlog
)
