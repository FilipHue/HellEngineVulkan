#include "hepch.h"
#include "glfw_window.h"

// Internal
#include <hellengine/core/events/event.h>
#include <hellengine/core/input/input.h>

// External
#include <imgui_impl_glfw.h>

namespace hellengine
{

	namespace platform
	{

		GlfwWindow::GlfwWindow(const WindowConfiguration& config) : Window(config)
		{
			i32 succesful_init = glfwInit();
			if (succesful_init == GLFW_FALSE)
			{
				HE_CORE_ERROR("Failed to initialize GLFW library");
				exit(EXIT_FAILURE);
			}
			HE_CORE_DEBUG("GLFW library initalized successfully");

			SetWindowHints();

			m_monitor = glfwGetPrimaryMonitor();
			m_vidmode = glfwGetVideoMode(m_monitor);

			m_handle = glfwCreateWindow(config.width, config.height, config.title, nullptr, nullptr);

			if (!m_handle)
			{
				HE_CORE_DEBUG("Failed to create window. Shutting down application");
				glfwTerminate();
				exit(EXIT_FAILURE);
			}

			SetWindowCallbacks();
			SetKeyCallbacks();
			SetMouseCallbacks();

			WindowsInternalState* internal_state = (WindowsInternalState*)calloc(1, sizeof(WindowsInternalState));
			if (!internal_state)
			{
				HE_CORE_ERROR("Failed to allocate memory for internal state");
				exit(EXIT_FAILURE);
			}
			else
			{
				internal_state->instance_handle = GetModuleHandleW(nullptr);
				internal_state->window_handle = glfwGetWin32Window(m_handle);
			}

			m_state.data = calloc(1, sizeof(WindowsInternalState));
			if (!m_state.data)
			{
				HE_CORE_ERROR("Failed to allocate memory for internal state");
				exit(EXIT_FAILURE);
			}
			else
			{
				memcpy(m_state.data, internal_state, sizeof(WindowsInternalState));
			}

			CenterWindow();
			CenterCursor();
		}

		GlfwWindow::~GlfwWindow()
		{
			glfwDestroyWindow(m_handle);
			glfwTerminate();
		}

		void* GlfwWindow::GetHandle()
		{
			return m_handle;
		}

		void GlfwWindow::PollEvents()
		{
			glfwPollEvents();
		}

		void GlfwWindow::UpdateInternalState()
		{
			NO_OP;
		}

		void GlfwWindow::SetCursorMode(u32 mode)
		{
			glfwSetInputMode(m_handle, GLFW_CURSOR, mode);
		}

		void GlfwWindow::SetTitle(const char* title)
		{
			glfwSetWindowTitle(m_handle, title);
		}

		void GlfwWindow::CenterWindow()
		{
			glfwSetWindowPos(m_handle, (m_vidmode->width - m_configuration.width) / 2, (m_vidmode->height - m_configuration.height) / 2);
		}

		void GlfwWindow::CenterCursor()
		{
			glfwSetCursorPos(m_handle, m_configuration.width / 2, m_configuration.height / 2);
		}

		void GlfwWindow::SetWindowHints()
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
			glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
			glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
			glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
		}

		void GlfwWindow::SetWindowCallbacks()
		{
			glfwSetWindowCloseCallback(m_handle, [](GLFWwindow* window)
				{
					EventContext event;
					event.type = EventType_WindowClose;
					EventDispatcher::Dispatch(event);
				});

			glfwSetWindowSizeCallback(m_handle, [](GLFWwindow* window, int width, int height)
				{
					EventContext event;
					event.type = EventType_WindowResize;
					event.data.window_resize.width = width;
					event.data.window_resize.height = height;
					EventDispatcher::Dispatch(event);
				});

			glfwSetWindowFocusCallback(m_handle, [](GLFWwindow* window, int focused)
				{
					ImGui_ImplGlfw_WindowFocusCallback(window, focused);

					EventContext event;
					event.type = EventType_WindowFocus;
					event.data.window_focus.focused = focused;
					EventDispatcher::Dispatch(event);
				});

			glfwSetWindowIconifyCallback(m_handle, [](GLFWwindow* window, int iconified)
				{
					EventContext event;
					event.type = EventType_WindowIconified;
					event.data.window_iconified.is_iconified = iconified;
					EventDispatcher::Dispatch(event);
				});

			glfwSetWindowPosCallback(m_handle, [](GLFWwindow* window, int xpos, int ypos)
				{
					EventContext event;
					event.type = EventType_WindowMoved;
					event.data.window_moved.x = xpos;
					event.data.window_moved.y = ypos;
					EventDispatcher::Dispatch(event);
				});
		}

		void GlfwWindow::SetKeyCallbacks()
		{
			glfwSetKeyCallback(m_handle, [](GLFWwindow* window, int key, int scancode, int action, int mods)
				{
					ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

					EventContext event;
					event.data.key_event.key = (keys)key;
					event.data.key_event.scancode = scancode;
					event.data.key_event.mods = mods;

					switch (action)
					{
					case GLFW_PRESS:
					{
						event.type = EventType_KeyPressed;
						event.data.key_event.is_pressed = true;
						EventDispatcher::Dispatch(event);
						Input::ProcessKey((keys)key, true);
						break;
					}
					case GLFW_RELEASE:
					{
						event.type = EventType_KeyReleased;
						event.data.key_event.is_pressed = false;
						EventDispatcher::Dispatch(event);

						Input::ProcessKey((keys)key, false);
						break;
					}
					default:
						break;
					}
				});

			glfwSetCharCallback(m_handle, [](GLFWwindow* window, unsigned int unicode_key)
				{
					ImGui_ImplGlfw_CharCallback(window, unicode_key);

					EventContext event;
					event.type = EventType_KeyTyped;
					event.data.key_event.key = (keys)unicode_key;
					EventDispatcher::Dispatch(event);
				});
		}

		void GlfwWindow::SetMouseCallbacks()
		{
			glfwSetMouseButtonCallback(m_handle, [](GLFWwindow* window, int button, int action, int mods)
				{
					ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

					EventContext event;
					event.data.mouse_button_event.button = (mouse_buttons)button;
					event.data.mouse_button_event.mods = mods;

					switch (action)
					{
					case GLFW_PRESS:
					{
						event.type = EventType_MouseButtonPressed;
						event.data.mouse_button_event.is_pressed = true;
						EventDispatcher::Dispatch(event);

						Input::ProcessMouseButton((mouse_buttons)(button), true);
						break;
					}
					case GLFW_RELEASE:
					{
						event.type = EventType_MouseButtonReleased;
						event.data.mouse_button_event.is_pressed = false;
						EventDispatcher::Dispatch(event);

						Input::ProcessMouseButton((mouse_buttons)(button), false);
						break;
					}
					default:
						break;
					}
				});

			glfwSetCursorPosCallback(m_handle, [](GLFWwindow* window, double xpos, double ypos)
				{
					ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

					EventContext event;
					event.type = EventType_MouseMoved;
					event.data.mouse_moved.x = (f32)xpos;
					event.data.mouse_moved.y = (f32)ypos;
					EventDispatcher::Dispatch(event);

					Input::ProcessMousePosition((f32)xpos, (f32)ypos);
				});

			glfwSetScrollCallback(m_handle, [](GLFWwindow* window, double xoffset, double yoffset)
				{
					ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

					EventContext event;
					event.type = EventType_MouseScrolled;
					event.data.mouse_scrolled.delta_x = (f32)xoffset;
					event.data.mouse_scrolled.delta_y = (f32)yoffset;
					EventDispatcher::Dispatch(event);

					Input::ProcessMouseScroll((f32)xoffset, (f32)yoffset);
				});
		}

	} // namespace platform

} // namespace hellengine