#pragma once

#include <khepri/exceptions.hpp>

#include <string>
#include <string_view>

namespace khepri::renderer {

/**
 * Base class for errors in the renderer
 */
class Error : public khepri::Error
{
public:
    using khepri::Error::Error;
};

/**
 * Class to indicate shader compilation errors
 */
class ShaderCompileError : public khepri::renderer::Error
{
public:
    /**
     * Constructs a shader_compile_error with a generic exception message and a detailed error
     * message
     * \param[in] message the generic exception message
     * \param[in] error_message the detailed compilation error message
     */
    ShaderCompileError(const char* message, std::string_view error_message)
        : Error(message), m_error_message(error_message)
    {}

    /// Returns the detailed compilation error message
    [[nodiscard]] std::string_view error_message() const noexcept
    {
        return m_error_message;
    }

private:
    std::string m_error_message;
};

} // namespace khepri::renderer
