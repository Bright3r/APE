#pragma once

#include "render/Shader.h"
#include "render/Mesh.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>

#include <memory>

class App {
public:
	// Template Methods
	void setup();
	void update();
	void draw();
	void onKeyDown(SDL_KeyboardEvent key);
	void onKeyUp(SDL_KeyboardEvent key);
	void onMouseDown(SDL_MouseButtonEvent mButton);
	void onMouseUp(SDL_MouseButtonEvent mButton);
	void onMouseMove(SDL_MouseMotionEvent mEvent);

private:
	// App Data
	std::unique_ptr<APE::Render::Shader> shader;
	APE::Render::Mesh mesh;

	/* 2D TRIANGLE MESH DATA */

	// std::vector<APE::Render::PositionColorVertex> vertex_data = {
	// 	{    -1,    -1, 0, 255,   0,   0, 255 },
	// 	{     1,    -1, 0,   0, 255,   0, 255 },
	// 	{     0,     1, 0,   0,   0, 255, 255 }
	// };
	//
	// std::vector<APE::Render::VertexIndex> index_data = {
	// 	0, 1, 2
	// };
	

	/* CUBE MESH DATA */

	// std::vector<APE::Render::PositionColorVertex> vertex_data = {
	// 	// Front face
	// 	{ -0.5, -0.5,  0.5, 255,   0,   0, 255 },	// 0
	// 	{  0.5, -0.5,  0.5,   0, 255,   0, 255 },	// 1
	// 	{  0.5,  0.5,  0.5,   0,   0, 255, 255 },	// 2
	// 	{ -0.5,  0.5,  0.5, 255, 255,   0, 255 },	// 3
	//
	// 	// Back face
	// 	{  0.5, -0.5, -0.5, 255,   0, 255, 255 },	// 4
	// 	{ -0.5, -0.5, -0.5,   0, 255, 255, 255 },	// 5
	// 	{ -0.5,  0.5, -0.5, 255, 255, 255, 255 },	// 6
	// 	{  0.5,  0.5, -0.5,   0,   0,   0, 255 },	// 7
	//
	// 	// Right face
	// 	{  0.5,  0.5,  0.5,   0,   0, 255, 255 }	// 8
	// };
	//
	//
	// std::vector<APE::Render::VertexIndex> index_data = {
	// 	// Front face
	// 	0, 1, 2,
	// 	0, 2, 3,
	//
	// 	// Back face
	// 	4, 5, 6,
	// 	4, 6, 7,
	//
	// 	// Left face
	// 	5, 0, 3,
	// 	5, 3, 6,
	//
	// 	// Right face
	// 	1, 4, 7,
	// 	1, 7, 8,
	//
	// 	// Top face
	// 	3, 2, 7,
	// 	3, 7, 6,
	//
	// 	// Bottom face
	// 	5, 4, 1,
	// 	5, 1, 0,
	// };
	


	/* PYRAMID MESH DATA */
	
	std::vector<APE::Render::PositionColorVertex> vertex_data = {
		{ -1, 0,  1, 255,   0,   0, 255 }, // 0 front-left
		{  1, 0,  1,   0, 255,   0, 255 }, // 1 front-right
		{  1, 0, -1,   0,   0, 255, 255 }, // 2 back-right
		{ -1, 0, -1, 255, 255,   0, 255 }, // 3 back-left
		{  0, 1,  0, 255, 255, 255, 255 }  // 4 apex
	};

	std::vector<APE::Render::VertexIndex> index_data = {
		// Base
		0, 2, 1,
		0, 3, 2,

		// Sides
		0, 1, 4, // front
		1, 2, 4, // right
		2, 3, 4, // back
		3, 0, 4  // left
	};

};

