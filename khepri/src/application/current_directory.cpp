#include <khepri/application/current_directory.hpp>
#include <khepri/exceptions.hpp>

#if defined(_MSC_VER)
#include <Windows.h>
#else
#include <cstdlib>
#include <unistd.h>
#endif

#include <array>
#include <string>

namespace khepri::application {
namespace {
struct FreeDeleter
{
    template <typename T>
    void operator()(T* p) const
    {
        // NOLINTNEXTLINE
        std::free(const_cast<std::remove_const_t<T>*>(p));
    }
};
} // namespace
std::filesystem::path get_current_directory()
{
#ifdef _MSC_VER
    std::array<CHAR, MAX_PATH> curdir{};
    if (GetCurrentDirectoryA(MAX_PATH, curdir.data()) == 0) {
        throw Error("Failed to get current directory");
    }
    return curdir.data();
#else
    const std::unique_ptr<char, FreeDeleter> curdir_(getcwd(nullptr, 0));
    if (curdir_ == nullptr) {
        throw Error("Failed to get current directory");
    }
    return curdir_.get();
#endif
}

} // namespace khepri::application
