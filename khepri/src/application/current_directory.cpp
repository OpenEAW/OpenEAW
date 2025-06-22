#include <khepri/application/current_directory.hpp>

#if defined(_MSC_VER)
#include <Windows.h>
#else
#include <cstdlib>
#include <unistd.h>
#endif

#include <array>
#include <string>

namespace khepri::application {

std::filesystem::path get_current_directory()
{
#ifdef _MSC_VER
    std::array<CHAR, MAX_PATH> curdir{};
    GetCurrentDirectoryA(MAX_PATH, curdir.data());
    return curdir.data();
#else
    char*                 curdir_ = getcwd(nullptr, 0);
    std::filesystem::path curdir  = curdir_;
    std::free(curdir_);
    return curdir;
#endif
}

} // namespace khepri::application
