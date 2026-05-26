#include "editor_settings.h"

void EditorSettings::Serialize()
{
	YAML::Node root;

	// Editor settings
	root["editor"]["show_grid"] = static_cast<b8>(show_grid);
	root["editor"]["show_gizmos"] = static_cast<b8>(show_gizmos);
	root["editor"]["render_mode"] = static_cast<i32>(render_mode);
	root["editor"]["geometry_mode"] = static_cast<i32>(geometry_mode);

	// Window settings
	SerializeWindowSettings(root);

	// Global Illumination settings
	SerializeGISettings(root);

	// Shadow settings
	SerializeShadowSettings(root);

	// Write to file
	FileManager::WriteFile(m_config_file, YAML::Dump(root));
}

void EditorSettings::Deserialize()
{
	LoadDefaults();

	// Try to load from file
	if (!FileManager::Exists(m_config_file))
	{
		// File doesn't exist, use defaults
		return;
	}

	// File exists, load settings
	YAML::Node root;
	root = YAML::LoadFile(m_config_file);

	// Window settings
	DeserializeWindowSettings(root);

	// Editor settings
	if (root["editor"])
	{
		if (root["editor"]["show_grid"]) show_grid = root["editor"]["show_grid"].as<b8>();
		if (root["editor"]["show_gizmos"]) show_gizmos = root["editor"]["show_gizmos"].as<b8>();
		if (root["editor"]["render_mode"]) render_mode = static_cast<EditorRenderMode>(root["editor"]["render_mode"].as<i32>());
		if (root["editor"]["geometry_mode"]) geometry_mode = static_cast<GeometryMode>(root["editor"]["geometry_mode"].as<i32>());

		// Global Illumination settings
		DeserializeGISettings(root);

		// Shadow settings
		DeserializeShadowSettings(root);
	}
}

void EditorSettings::LoadDefaults()
{
	// Load defaults first
	{
		// Window settings
		window_settings = {};
		window_settings.show_hierarchy = true;
		window_settings.show_inspector = true;
		window_settings.show_editor_settings = false;
		window_settings.show_render_settings = false;
		window_settings.show_gi_settings = false;
		window_settings.show_shadow_settings = false;
		window_settings.show_about_window = false;
	}

	// Editor settings
	{
		show_grid = true;
		show_gizmos = true;
		render_mode = EditorRenderMode_Normal;
		geometry_mode = GeometryMode_Classic;

		// Initialize Global Illumination settings
		gi_settings = {};
		gi_settings.enabled = 0;
		gi_settings.sample_count = 16;
		gi_settings.ray_distance = 5.0f;
		gi_settings.intensity = 1.0f;
		gi_settings.thickness = 0.5f;
		gi_settings.falloff = 2.0f;
		gi_settings.bias = 0.05f;
		gi_settings.temporal_weight = 0.95f;
		gi_settings.debug_visualization = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

		// Initialize Shadow settings
		shadow_settings = {};
		shadow_settings.enabled = 1;
		shadow_settings.shadow_map_size = 4096;
		shadow_settings.cascade_split_lambda = 0.75f;
		shadow_settings.min_bias = 0.001f;
		shadow_settings.max_bias = 0.01f;
		shadow_settings.normal_offset = 0.02f;
		shadow_settings.pcf_samples = 1;
		shadow_settings.softness = 1.0f;
		shadow_settings.cascade_distances = glm::vec4(10.0f, 30.0f, 300.0f, 1000.0f);
	}
}

void EditorSettings::SerializeGISettings(YAML::Node& root)
{
	root["editor"]["gi_settings"]["enabled"] = gi_settings.enabled;
	root["editor"]["gi_settings"]["sample_count"] = gi_settings.sample_count;
	root["editor"]["gi_settings"]["ray_distance"] = gi_settings.ray_distance;
	root["editor"]["gi_settings"]["intensity"] = gi_settings.intensity;
	root["editor"]["gi_settings"]["thickness"] = gi_settings.thickness;
	root["editor"]["gi_settings"]["falloff"] = gi_settings.falloff;
	root["editor"]["gi_settings"]["bias"] = gi_settings.bias;
	root["editor"]["gi_settings"]["temporal_weight"] = gi_settings.temporal_weight;
	root["editor"]["gi_settings"]["debug_visualization"] = gi_settings.debug_visualization;
}

void EditorSettings::SerializeShadowSettings(YAML::Node& root)
{
	root["editor"]["shadow_settings"]["enabled"] = shadow_settings.enabled;
	root["editor"]["shadow_settings"]["shadow_map_size"] = shadow_settings.shadow_map_size;
	root["editor"]["shadow_settings"]["cascade_split_lambda"] = shadow_settings.cascade_split_lambda;
	root["editor"]["shadow_settings"]["min_bias"] = shadow_settings.min_bias;
	root["editor"]["shadow_settings"]["max_bias"] = shadow_settings.max_bias;
	root["editor"]["shadow_settings"]["normal_offset"] = shadow_settings.normal_offset;
	root["editor"]["shadow_settings"]["pcf_samples"] = shadow_settings.pcf_samples;
	root["editor"]["shadow_settings"]["softness"] = shadow_settings.softness;
	root["editor"]["shadow_settings"]["cascade_distances"] = shadow_settings.cascade_distances;
}

void EditorSettings::SerializeWindowSettings(YAML::Node& root)
{
	root["window"]["show_hierarchy"] = window_settings.show_hierarchy;
	root["window"]["show_inspector"] = window_settings.show_inspector;
	root["window"]["show_editor_settings"] = window_settings.show_editor_settings;
	root["window"]["show_render_settings"] = window_settings.show_render_settings;
	root["window"]["show_gi_settings"] = window_settings.show_gi_settings;
	root["window"]["show_shadow_settings"] = window_settings.show_shadow_settings;
	root["window"]["show_about_window"] = window_settings.show_about_window;
}

void EditorSettings::DeserializeGISettings(const YAML::Node& root)
{
	if (root["editor"]["gi_settings"])
	{
		if (root["editor"]["gi_settings"]["enabled"])
			gi_settings.enabled = root["editor"]["gi_settings"]["enabled"].as<i32>();
		if (root["editor"]["gi_settings"]["sample_count"])
			gi_settings.sample_count = root["editor"]["gi_settings"]["sample_count"].as<i32>();
		if (root["editor"]["gi_settings"]["ray_distance"])
			gi_settings.ray_distance = root["editor"]["gi_settings"]["ray_distance"].as<float>();
		if (root["editor"]["gi_settings"]["intensity"])
			gi_settings.intensity = root["editor"]["gi_settings"]["intensity"].as<float>();
		if (root["editor"]["gi_settings"]["thickness"])
			gi_settings.thickness = root["editor"]["gi_settings"]["thickness"].as<float>();
		if (root["editor"]["gi_settings"]["falloff"])
			gi_settings.falloff = root["editor"]["gi_settings"]["falloff"].as<float>();
		if (root["editor"]["gi_settings"]["bias"])
			gi_settings.bias = root["editor"]["gi_settings"]["bias"].as<float>();
		if (root["editor"]["gi_settings"]["temporal_weight"])
			gi_settings.temporal_weight = root["editor"]["gi_settings"]["temporal_weight"].as<float>();

		if (root["editor"]["gi_settings"]["debug_visualization"])
			gi_settings.debug_visualization = root["editor"]["gi_settings"]["debug_visualization"].as<glm::vec4>();
	}
}

void EditorSettings::DeserializeShadowSettings(const YAML::Node& root)
{
	if (root["editor"]["shadow_settings"])
	{
		if (root["editor"]["shadow_settings"]["enabled"])
			shadow_settings.enabled = root["editor"]["shadow_settings"]["enabled"].as<i32>();
		if (root["editor"]["shadow_settings"]["shadow_map_size"])
			shadow_settings.shadow_map_size = root["editor"]["shadow_settings"]["shadow_map_size"].as<i32>();
		if (root["editor"]["shadow_settings"]["cascade_split_lambda"])
			shadow_settings.cascade_split_lambda = root["editor"]["shadow_settings"]["cascade_split_lambda"].as<float>();
		if (root["editor"]["shadow_settings"]["min_bias"])
			shadow_settings.min_bias = root["editor"]["shadow_settings"]["min_bias"].as<float>();
		if (root["editor"]["shadow_settings"]["max_bias"])
			shadow_settings.max_bias = root["editor"]["shadow_settings"]["max_bias"].as<float>();
		if (root["editor"]["shadow_settings"]["normal_offset"])
			shadow_settings.normal_offset = root["editor"]["shadow_settings"]["normal_offset"].as<float>();
		if (root["editor"]["shadow_settings"]["pcf_samples"])
			shadow_settings.pcf_samples = root["editor"]["shadow_settings"]["pcf_samples"].as<i32>();
		if (root["editor"]["shadow_settings"]["softness"])
			shadow_settings.softness = root["editor"]["shadow_settings"]["softness"].as<float>();

		if (root["editor"]["shadow_settings"]["cascade_distances"])
			shadow_settings.cascade_distances = root["editor"]["shadow_settings"]["cascade_distances"].as<glm::vec4>();
	}
}

void EditorSettings::DeserializeWindowSettings(const YAML::Node& root)
{
	if (root["window"])
	{
		if (root["window"]["show_hierarchy"]) window_settings.show_hierarchy = root["window"]["show_hierarchy"].as<b8>();
		if (root["window"]["show_inspector"]) window_settings.show_inspector = root["window"]["show_inspector"].as<b8>();
		if (root["window"]["show_editor_settings"]) window_settings.show_editor_settings = root["window"]["show_editor_settings"].as<b8>();
		if (root["window"]["show_render_settings"]) window_settings.show_render_settings = root["window"]["show_render_settings"].as<b8>();
		if (root["window"]["show_gi_settings"]) window_settings.show_gi_settings = root["window"]["show_gi_settings"].as<b8>();
		if (root["window"]["show_shadow_settings"]) window_settings.show_shadow_settings = root["window"]["show_shadow_settings"].as<b8>();
		if (root["window"]["show_about_window"]) window_settings.show_about_window = root["window"]["show_about_window"].as<b8>();
	}
}
