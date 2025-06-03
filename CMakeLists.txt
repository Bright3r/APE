cmake_minimum_required(VERSION 3.20)

project(ape)

find_package(SDL3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(ape src/main.cpp)

target_link_libraries(ape PRIVATE 
	SDL3::SDL3
	glad::glad
	glm::glm
)
