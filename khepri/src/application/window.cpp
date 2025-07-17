#include <khepri/application/window.hpp>
#include <khepri/log/log.hpp>

#ifdef _MSC_VER
#define GLFW_EXPOSE_NATIVE_WIN32
#elif __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#else
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
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
#if defined(_MSC_VER) || defined(__APPLE__)
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#else
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
#endif
        return glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, title.c_str(), nullptr, nullptr);
    }

public:
    Impl(const std::string& title) : m_window(create_window(title))
    {
        // Make the window's context current on the current thread. This is required for renderers
        // to pick up the current context. This does introduce the constraint that the rendering
        // logic must run from the same thread that created the window.
        glfwMakeContextCurrent(m_window);

        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, framebuffer_size_changed);
        glfwSetCursorPosCallback(m_window, cursor_position_callback);
        glfwSetMouseButtonCallback(m_window, mouse_button_callback);
        glfwSetScrollCallback(m_window, mouse_scroll_callback);
#ifdef _MSC_VER
        LOG.info("Created window: {}; hWnd: {}", (void*)m_window,
                 (void*)glfwGetWin32Window(m_window));
#elif __APPLE__
        LOG.info("Created window: {}; NSWindow: {}", (void*)m_window, glfwGetCocoaWindow(m_window);
#else
        LOG.info("Created window: {}; X11 display; {}, X11 window: {:#x}", (void*)m_window,
                 static_cast<void*>(glfwGetX11Display()), glfwGetX11Window(m_window));
#endif
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

    [[nodiscard]] std::any native_handle() const
    {
#ifdef _MSC_VER
        return glfwGetWin32Window(m_window);
#elif __APPLE__
        return glfwGetCocoaWindow(m_window);
#else
        return std::tuple<void*, std::uint32_t>(glfwGetX11Display(), glfwGetX11Window(m_window));
#endif
    }

    [[nodiscard]] Size render_size() const
    {
        int width  = 0;
        int height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        return {static_cast<unsigned long>(width), static_cast<unsigned long>(height)};
    }

    [[nodiscard]] bool should_close() const
    {
        return glfwWindowShouldClose(m_window) == GLFW_TRUE;
    }

    [[nodiscard]] static bool use_swap_buffers()
    {
#if defined(_MSC_VER) || __APPLE__
        return false;
#else
        // For OpenGL, the window should be used to swap render buffers.
        return true;
#endif
    }

    void swap_buffers()
    {
        glfwSwapBuffers(m_window);
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

    void set_cursor_position(const Pointi& position)
    {
        glfwSetCursorPos(m_window, position.x, position.y);
    }

    void set_infinite_cursor(bool infinite)
    {
        glfwSetInputMode(m_window, GLFW_CURSOR,
                         infinite ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
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

    static void mouse_button_callback(GLFWwindow* w, int button, int action, int mods)
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
            case GLFW_MOUSE_BUTTON_MIDDLE:
                mb = MouseButton::middle;
                break;
            default:
                // We don't care about this button
                return;
            }

            const auto mba =
                (action == GLFW_PRESS) ? MouseButtonAction::pressed : MouseButtonAction::released;
            const auto mbm = convert_mods(mods);

            for (const auto& listener : window->m_mouse_button_listeners) {
                listener(window->m_cursor_pos, mb, mba, mbm);
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

    static KeyModifiers convert_mods(int mods)
    {
        KeyModifiers result = KeyModifiers::none;
        if (mods & GLFW_MOD_ALT) {
            result |= KeyModifiers::alt;
        }
        if (mods & GLFW_MOD_SHIFT) {
            result |= KeyModifiers::shift;
        }
        if (mods & GLFW_MOD_CONTROL) {
            result |= KeyModifiers::ctrl;
        }
        return result;
    }

    GLFWwindow*                         m_window;
    std::vector<SizeListener>           m_size_listeners;
    std::vector<CursorPositionListener> m_cursor_position_listeners;
    std::vector<MouseButtonListener>    m_mouse_button_listeners;
    std::vector<MouseScrollListener>    m_mouse_scroll_listeners;

    khepri::Pointi m_cursor_pos{0, 0};
};

Window::Window(const std::string& title) : m_impl(std::make_unique<Impl>(title)) {}

Window::~Window() = default;

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

bool Window::use_swap_buffers()
{
    return Impl::use_swap_buffers();
}

void Window::swap_buffers()
{
    m_impl->swap_buffers();
}

void Window::add_size_listener(const SizeListener& listener)
{
    m_impl->add_size_listener(listener);
}

void Window::add_cursor_position_listener(const CursorPositionListener& listener)
{
    m_impl->add_cursor_position_listener(listener);
}

void Window::set_cursor_position(const Pointi& position)
{
    m_impl->set_cursor_position(position);
}

void Window::set_infinite_cursor(bool infinite)
{
    m_impl->set_infinite_cursor(infinite);
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