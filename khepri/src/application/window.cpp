#include <khepri/application/window.hpp>
#include <khepri/log/log.hpp>

#ifdef _MSC_VER
#define GLFW_EXPOSE_NATIVE_WIN32
#else
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <gsl/gsl-lite.hpp>

#include <vector>

namespace khepri::application {
namespace {
constexpr log::Logger LOG("window");
} // namespace

class Window::Impl
{
    static constexpr auto WINDOW_WIDTH  = 1024;
    static constexpr auto WINDOW_HEIGHT = 768;

    static auto create_window(const std::string& title)
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        return glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, title.c_str(), nullptr, nullptr);
    }

public:
    Impl(const std::string& title) : m_window(create_window(title))
    {
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, framebuffer_size_changed);
        glfwSetCursorPosCallback(m_window, cursor_position_callback);
        glfwSetMouseButtonCallback(m_window, mouse_button_callback);
        glfwSetScrollCallback(m_window, mouse_scroll_callback);
        LOG.info("Created window: {}", (void*)m_window);
    }

    Impl(const Impl&)            = delete;
    Impl(Impl&&)                 = delete;
    Impl& operator=(const Impl&) = delete;
    Impl& operator=(Impl&&)      = delete;

    ~Impl()
    {
        glfwSetWindowUserPointer(m_window, nullptr);
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    std::any native_handle() const
    {
#ifdef _MSC_VER
        return glfwGetWin32Window(m_window);
#else
        return glfwGetCocoaWindow(m_window);
#endif
    }

    Size render_size() const
    {
        int width  = 0;
        int height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        return {static_cast<unsigned long>(width), static_cast<unsigned long>(height)};
    }

    bool should_close() const
    {
        return glfwWindowShouldClose(m_window) == GLFW_TRUE;
    }

    void add_size_listener(const SizeListener& listener)
    {
        m_size_listeners.push_back(listener);
    }

    void add_cursor_position_listener(const CursorPositionListener& listener)
    {
        m_cursor_position_listeners.push_back(listener);
    }

    void add_mouse_button_listener(const MouseButtonListener& listener)
    {
        m_mouse_button_listeners.push_back(listener);
    }

    void add_mouse_scroll_listener(const MouseScrollListener& listener)
    {
        m_mouse_scroll_listeners.push_back(listener);
    }

private:
    static Impl* get_window(GLFWwindow* glfw_window)
    {
        auto* data = glfwGetWindowUserPointer(glfw_window);
        return reinterpret_cast<Window::Impl*>(data); // NOLINT
    }

    static void framebuffer_size_changed(GLFWwindow* w, int /*width*/, int /*height*/)
    {
        auto* window = get_window(w);
        if (window != nullptr) {
            for (const auto& listener : window->m_size_listeners) {
                listener();
            }
        }
    }

    static void cursor_position_callback(GLFWwindow* w, double xpos, double ypos)
    {
        auto* window = get_window(w);
        if (window != nullptr) {
            window->m_cursor_pos = {static_cast<long>(xpos), static_cast<long>(ypos)};
            for (const auto& listener : window->m_cursor_position_listeners) {
                listener(window->m_cursor_pos);
            }
        }
    }

    static void mouse_button_callback(GLFWwindow* w, int button, int action, int /*mods*/)
    {
        auto* window = get_window(w);
        if (window != nullptr) {
            MouseButton mb{};
            switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                mb = MouseButton::left;
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                mb = MouseButton::right;
                break;
            default:
                // We don't care about this button
                return;
            }

            const auto mba =
                (action == GLFW_PRESS) ? MouseButtonAction::pressed : MouseButtonAction::released;

            for (const auto& listener : window->m_mouse_button_listeners) {
                listener(window->m_cursor_pos, mb, mba);
            }
        }
    }

    static void mouse_scroll_callback(GLFWwindow* w, double xoffset, double yoffset)
    {
        auto* window = get_window(w);
        if (window != nullptr) {
            for (const auto& listener : window->m_mouse_scroll_listeners) {
                listener(window->m_cursor_pos,
                         {static_cast<float>(xoffset), static_cast<float>(yoffset)});
            }
        }
    }

    GLFWwindow*                         m_window;
    std::vector<SizeListener>           m_size_listeners;
    std::vector<CursorPositionListener> m_cursor_position_listeners;
    std::vector<MouseButtonListener>    m_mouse_button_listeners;
    std::vector<MouseScrollListener>    m_mouse_scroll_listeners;

    khepri::Pointi m_cursor_pos{0, 0};
};

Window::Window(const std::string& title) : m_impl(std::make_unique<Impl>(title)) {}

Window::~Window() {}

std::any Window::native_handle() const
{
    return m_impl->native_handle();
}

Size Window::render_size() const
{
    return m_impl->render_size();
}

bool Window::should_close() const
{
    return m_impl->should_close();
}

void Window::add_size_listener(const SizeListener& listener)
{
    m_impl->add_size_listener(listener);
}

void Window::add_cursor_position_listener(const CursorPositionListener& listener)
{
    m_impl->add_cursor_position_listener(listener);
}

void Window::add_mouse_button_listener(const MouseButtonListener& listener)
{
    m_impl->add_mouse_button_listener(listener);
}

void Window::add_mouse_scroll_listener(const MouseScrollListener& listener)
{
    m_impl->add_mouse_scroll_listener(listener);
}

void Window::poll_events()
{
    glfwPollEvents();
}

} // namespace khepri::application