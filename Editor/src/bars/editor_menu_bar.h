#pragma once

// Internal
#include "hellengine/hellengine.h"

using namespace hellengine;
using namespace core;
using namespace ecs;
using namespace graphics;
using namespace ui;
using namespace math;
using namespace resources;

class EditorMenuBar
{
public:
	EditorMenuBar() = default;
	virtual ~EditorMenuBar() = default;

	void Init();

	void Draw();

private:
};