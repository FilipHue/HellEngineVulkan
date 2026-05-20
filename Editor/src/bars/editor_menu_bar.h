#pragma once

// Internal
#include "hellengine/hellengine.h"

#include "../editor_settings.h"
#include "../panels/editor_hierarchy.h"

using namespace hellengine;
using namespace core;
using namespace ecs;
using namespace graphics;
using namespace ui;
using namespace math;
using namespace resources;


// GI Samples
static constexpr i32 GI_SAMPLE_COUNT_MIN = 4;
static constexpr i32 GI_SAMPLE_COUNT_MAX = 256;

// GI Ambient Intensity
static constexpr f32 GI_AMBIENT_INTENSITY_MIN = 0.0f;
static constexpr f32 GI_AMBIENT_INTENSITY_MAX = 2.0f;
static constexpr f32 GI_AMBIENT_INTENSITY_STEP = 0.01f;

// GI Intensity
static constexpr f32 GI_INTENSITY_MIN = 0.0f;
static constexpr f32 GI_INTENSITY_MAX = 2.0f;
static constexpr f32 GI_INTENSITY_STEP = 0.01f;

// GI Ray Distance
static constexpr f32 GI_RAY_DISTANCE_MIN = 1.0f;
static constexpr f32 GI_RAY_DISTANCE_MAX = 20.0f;
static constexpr f32 GI_RAY_DISTANCE_STEP = 0.1f;

// GI Thickness
static constexpr f32 GI_THICKNESS_MIN = 0.1f;
static constexpr f32 GI_THICKNESS_MAX = 2.0f;
static constexpr f32 GI_THICKNESS_STEP = 0.01f;

// GI Falloff
static constexpr f32 GI_FALLOFF_MIN = 1.0f;
static constexpr f32 GI_FALLOFF_MAX = 4.0f;
static constexpr f32 GI_FALLOFF_STEP = 0.01f;

// Shadow Map Sizes
static constexpr u32 SHADOW_MAP_SIZE_512 = 512;
static constexpr u32 SHADOW_MAP_SIZE_1024 = 1024;
static constexpr u32 SHADOW_MAP_SIZE_2048 = 2048;
static constexpr u32 SHADOW_MAP_SIZE_4096 = 4096;

// Shadow Bias Settings
static constexpr f32 SHADOW_MIN_BIAS_MIN = 0.0f;
static constexpr f32 SHADOW_MIN_BIAS_MAX = 0.01f;
static constexpr f32 SHADOW_MIN_BIAS_STEP = 0.0001f;

static constexpr f32 SHADOW_MAX_BIAS_MIN = 0.001f;
static constexpr f32 SHADOW_MAX_BIAS_MAX = 0.1f;
static constexpr f32 SHADOW_MAX_BIAS_STEP = 0.001f;

// Normal Offset
static constexpr f32 SHADOW_NORMAL_OFFSET_MIN = 0.0f;
static constexpr f32 SHADOW_NORMAL_OFFSET_MAX = 1.0f;
static constexpr f32 SHADOW_NORMAL_OFFSET_STEP = 0.01f;

// PCF Samples
static constexpr i32 SHADOW_PCF_SAMPLES_MIN = 0;
static constexpr i32 SHADOW_PCF_SAMPLES_MAX = 4;
static constexpr f32 SHADOW_PCF_SAMPLES_STEP = 0.1f;

// Shadow Softness
static constexpr f32 SHADOW_SOFTNESS_MIN = 0.0f;
static constexpr f32 SHADOW_SOFTNESS_MAX = 5.0f;
static constexpr f32 SHADOW_SOFTNESS_STEP = 0.1f;

// UI Window Sizes
static constexpr f32 MENU_WINDOW_WIDTH = 400.0f;
static constexpr f32 MENU_WINDOW_HEIGHT = 0.0f;

// Menu Spacing Multipliers
static constexpr f32 MENU_BAR_ITEM_SPACING_X_MULTIPLIER = 2.0f;
static constexpr f32 MENU_ITEM_SPACING_Y_MULTIPLIER = 3.0f;

class EditorMenuBar
{
public:
	EditorMenuBar() = default;
	virtual ~EditorMenuBar() = default;

	void Init(EditorHierarchy* hierarchy, EditorSettings* settings);

	void Draw();

	void SetImportAssetCallback(const std::function<void()>& callback);
	void SetCreateEmptyGameObjectCallback(const std::function<void()>& callback);

private:
	void FileMenu(ImGuiStyle& style);
	void EditMenu(ImGuiStyle& style);
	void AssetsMenu(ImGuiStyle& style);
	void GameObjectMenu(ImGuiStyle& style);
	void ComponentMenu(ImGuiStyle& style);
	void WindowMenu(ImGuiStyle& style);
	void HelpMenu(ImGuiStyle& style);

	void ShowEditorSettings();
	void ShowRenderSettings();
	void ShowGISettings();
	void ShowShadowSettings();
	void ShowAboutWindow();

	void MenuSpacing(ImGuiStyle& style);

	EditorSettings* m_settings = nullptr;
	EditorHierarchy* m_hierarchy_panel = nullptr;

	std::function<void()> m_import_asset_callback;
	std::function<void()> m_create_empty_gameobject_callback;
};
