#pragma once

// Internal
#include "hellengine/hellengine.h"

using namespace hellengine;
using namespace core;
using namespace graphics;
using namespace ui;
using namespace math;
using namespace resources;

class EditorHierarchy : public Panel
{
public:
	EditorHierarchy();
	virtual ~EditorHierarchy() = default;

	void Init();
	void Draw() override;

private:
};