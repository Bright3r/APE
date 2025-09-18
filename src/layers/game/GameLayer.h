#pragma once

#include "core/Application.h"

namespace APE::Game {

class GameLayer : public Application {

public:
	void draw() noexcept override;
	void setup() noexcept override;
	void update() noexcept override;
	void drawGUI() noexcept override;
};

};	// end of namespace
