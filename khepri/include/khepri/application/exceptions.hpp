#pragma once

#include <functional>
#include <optional>
#include <string>
#include <type_traits>

namespace khepri::application {

/**
 * @brief Handle all exceptions thrown from a callable.
 *
 * This class allows generic callables to be invoked, but wraps the call in both normal C++ and
 * platform-native exception handling. When such exceptions occur, debugging information is written
 * out to the console.
 */
class ExceptionHandler
{
public:
    /**
     * Constructs a new Exception Handler object for a given context.
     *
     * The context is used when handling exceptions to identify where the exception occured, in case
     * multiple ExceptionHandlers are instantiated.
     */
    explicit ExceptionHandler(std::string context);

    /**
     * Invokes the callable and handles any exceptions it throws.
     *
     * If the callable returns @a void, this method returns @a bool: @a true if the code ran
     * succesfully, or @a false if an exception was handled.
     *
     * If the callable returns any other type, this method returns a @a std::optional of that type,
     * where @a std::nullopt is returned if an exception was thrown, and otherwise the return value
     * of the callable.
     */
    template <typename Callable>
    auto invoke(const Callable& callable)
    {
        using ResultType = decltype(callable());
        if constexpr (std::is_same_v<ResultType, void>) {
            return invoke_void(callable);
        } else {
            std::optional<ResultType> result{};
            invoke_void([&] { result = callable(); });
            return result;
        }
    }

private:
    bool invoke_void(const std::function<void()>& callable);

    std::string m_context;
};

} // namespace khepri::application