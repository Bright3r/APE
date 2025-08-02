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
	void draw(SDL_GPURenderPass *render_pass);
	void onKeyDown(SDL_KeyboardEvent key);
	void onKeyUp(SDL_KeyboardEvent key);
	void onMouseDown(SDL_MouseButtonEvent mButton);
	void onMouseUp(SDL_MouseButtonEvent mButton);
	void onMouseMove(SDL_MouseMotionEvent mEvent);

private:
	// App Data
	std::unique_ptr<APE::Render::Shader> shader;
	APE::Render::Mesh mesh;

	// std::vector<PositionColorVertex> vertex_data = {
	// 	{    -1,    -1, 0, 255,   0,   0, 255 },
	// 	{     1,    -1, 0,   0, 255,   0, 255 },
	// 	{     0,     1, 0,   0,   0, 255, 255 }
	// };

	std::vector<APE::Render::PositionColorVertex> vertex_data = {
		// Front face (z = +0.5)
		{ -0.5, -0.5,  0.5, 255,   0,   0, 255 }, // Bottom-left
		{  0.5, -0.5,  0.5,   0, 255,   0, 255 }, // Bottom-right
		{  0.5,  0.5,  0.5,   0,   0, 255, 255 }, // Top-right

		{ -0.5, -0.5,  0.5, 255,   0,   0, 255 }, // Bottom-left
		{  0.5,  0.5,  0.5,   0,   0, 255, 255 }, // Top-right
		{ -0.5,  0.5,  0.5, 255, 255,   0, 255 }, // Top-left

		// Back face (z = -0.5)
		{  0.5, -0.5, -0.5, 255,   0, 255, 255 }, // Bottom-right
		{ -0.5, -0.5, -0.5,   0, 255, 255, 255 }, // Bottom-left
		{ -0.5,  0.5, -0.5, 255, 255, 255, 255 }, // Top-left

		{  0.5, -0.5, -0.5, 255,   0, 255, 255 }, // Bottom-right
		{ -0.5,  0.5, -0.5, 255, 255, 255, 255 }, // Top-left
		{  0.5,  0.5, -0.5,   0,   0,   0, 255 }, // Top-right

		// Left face (x = -0.5)
		{ -0.5, -0.5, -0.5,   0, 255, 255, 255 }, // Bottom-back
		{ -0.5, -0.5,  0.5, 255,   0,   0, 255 }, // Bottom-front
		{ -0.5,  0.5,  0.5, 255, 255,   0, 255 }, // Top-front

		{ -0.5, -0.5, -0.5,   0, 255, 255, 255 }, // Bottom-back
		{ -0.5,  0.5,  0.5, 255, 255,   0, 255 }, // Top-front
		{ -0.5,  0.5, -0.5, 255, 255, 255, 255 }, // Top-back

		// Right face (x = +0.5)
		{  0.5, -0.5,  0.5,   0, 255,   0, 255 }, // Bottom-front
		{  0.5, -0.5, -0.5, 255,   0, 255, 255 }, // Bottom-back
		{  0.5,  0.5, -0.5,   0,   0,   0, 255 }, // Top-back

		{  0.5, -0.5,  0.5,   0, 255,   0, 255 }, // Bottom-front
		{  0.5,  0.5, -0.5,   0,   0,   0, 255 }, // Top-back
		{  0.5,  0.5,  0.5,   0,   0, 255, 255 }, // Top-front

		// Top face (y = +0.5)
		{ -0.5,  0.5,  0.5, 255, 255,   0, 255 }, // Front-left
		{  0.5,  0.5,  0.5,   0,   0, 255, 255 }, // Front-right
		{  0.5,  0.5, -0.5,   0,   0,   0, 255 }, // Back-right

		{ -0.5,  0.5,  0.5, 255, 255,   0, 255 }, // Front-left
		{  0.5,  0.5, -0.5,   0,   0,   0, 255 }, // Back-right
		{ -0.5,  0.5, -0.5, 255, 255, 255, 255 }, // Back-left

		// Bottom face (y = -0.5)
		{ -0.5, -0.5, -0.5,   0, 255, 255, 255 }, // Back-left
		{  0.5, -0.5, -0.5, 255,   0, 255, 255 }, // Back-right
		{  0.5, -0.5,  0.5,   0, 255,   0, 255 }, // Front-right

		{ -0.5, -0.5, -0.5,   0, 255, 255, 255 }, // Back-left
		{  0.5, -0.5,  0.5,   0, 255,   0, 255 }, // Front-right
		{ -0.5, -0.5,  0.5, 255,   0,   0, 255 }, // Front-left
	};

};

