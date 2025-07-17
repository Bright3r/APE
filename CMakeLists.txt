cmake_minimum_required(VERSION 3.20)

project(ape)

find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(ape 
	src/main.cpp
	src/App.cpp
	src/AppRunner.cpp
	src/render/Shader.cpp
	src/render/Context.cpp
	src/render/Renderer.cpp
)

target_include_directories(ape PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(ape PRIVATE 
	SDL3::SDL3
	glm::glm
)
