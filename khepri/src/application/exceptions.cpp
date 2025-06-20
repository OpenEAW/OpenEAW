#include <khepri/application/exceptions.hpp>
#include <khepri/log/log.hpp>

#include <fmt/ranges.h>
#include <gsl/gsl-lite.hpp>

#include <exception>
#include <utility>

#if defined(_MSC_VER)
#include <Windows.h>
#ifndef NDEBUG
#include <dbghelp.h>
#include <debugapi.h>
#endif
#endif

namespace khepri::application {
namespace {

constexpr log::Logger LOG("application");

#ifdef _MSC_VER
void print_native_exception(const std::string& context, HANDLE hProcess,
                            EXCEPTION_POINTERS* exc_ptrs)
{
    if (exc_ptrs == nullptr || exc_ptrs->ExceptionRecord == nullptr) {
        LOG.error("Caught unknown native exception in '{}'", context);
    } else {
        const auto* exceptr        = exc_ptrs->ExceptionRecord;
        auto*       context_record = exc_ptrs->ContextRecord;

        gsl::span<const ULONG_PTR> params(&exceptr->ExceptionInformation[0],
                                          exceptr->NumberParameters);

        LOG.error(
            "Caught native exception in '{}': code={:#x}, flags={}, address={:p}, params={{{:#x}}}",
            context, exceptr->ExceptionCode, exceptr->ExceptionFlags, exceptr->ExceptionAddress,
            fmt::join(params, ", "));
#ifndef NDEBUG
        DWORD machine_type = 0;
#if defined(_M_IX86)
        machine_type = IMAGE_FILE_MACHINE_I386;
#elif defined(_M_IA64)
        machine_type = IMAGE_FILE_MACHINE_IA64;
#elif defined(_M_AMD64) || defined(_M_X64)
        machine_type = IMAGE_FILE_MACHINE_AMD64;
#endif
        LOG.error("Stack trace:");

        STACKFRAME frame{};
        while (StackWalk64(machine_type, hProcess, GetCurrentThread(), &frame, context_record,
                           nullptr, SymFunctionTableAccess64, SymGetModuleBase64,
                           nullptr) != FALSE) {
            std::array<char, sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)> buffer{};
            auto* pSymbol         = reinterpret_cast<SYMBOL_INFO*>(buffer.data()); // NOLINT
            pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            pSymbol->MaxNameLen   = MAX_SYM_NAME;

            DWORD64         displacement = 0;
            IMAGEHLP_LINE64 line;
            line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

            if (SymFromAddr(hProcess, frame.AddrPC.Offset, &displacement, pSymbol) != FALSE) {
                auto symbol_name = std::string_view(&pSymbol->Name[0], pSymbol->NameLen);
                if (SymGetLineFromAddr64(hProcess, frame.AddrPC.Offset, nullptr, &line) != FALSE) {
                    LOG.error(" - {:#018x} {} + {} ({}:{})", frame.AddrPC.Offset, symbol_name,
                              displacement, line.FileName, line.LineNumber);
                } else {
                    LOG.error(" - {:#018x} {} + {}", frame.AddrPC.Offset, symbol_name,
                              displacement);
                }
            } else {
                LOG.error(" - {:#018x}", frame.AddrPC.Offset);
            }
        }
#endif
    }
}

template <typename TCallable>
bool handle_native_exceptions(const std::string& context, TCallable callable)
{
#ifndef NDEBUG
    // Don't catch exceptions in debug builds if a debugger is attached -- the debugger will catch
    // them instead
    if (IsDebuggerPresent()) {
        return callable();
    }
#endif

    HANDLE hProcess = GetCurrentProcess();
#ifndef NDEBUG
    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS);
    SymInitialize(hProcess, nullptr, TRUE);
#endif
    bool result = false;
    __try {
        result = callable();
        // NOLINTNEXTLINE -- GetExceptionInformation is a macro and confuses clang-tidy
    } __except (print_native_exception(context, hProcess, GetExceptionInformation()),
                EXCEPTION_EXECUTE_HANDLER) {
    }
#ifndef NDEBUG
    SymCleanup(hProcess);
#endif
    return result;
}
#else
template <typename TCallable>
void handle_native_exceptions(const std::string& /*context*/, TCallable callable)
{
    callable();
}
#endif

template <typename TCallable>
bool handle_language_exceptions(const std::string& context, TCallable callable)
{
#ifndef NDEBUG
    // Don't catch exceptions in debug builds if a debugger is attached -- the debugger will catch
    // them instead
    if (IsDebuggerPresent()) {
        return callable();
    }
#endif

    try {
        return callable();
    } catch (const std::exception& e) {
        LOG.error("Caught unhandled exception in '{}': {}", context, e.what());
#ifdef _MSC_VER
        MessageBoxA(nullptr, e.what(), nullptr, MB_OK);
#endif
    } catch (...) {
        LOG.error("Caught unhandled unknown exception in '{}'", context);
    }
    return false;
}

class ScopedTerminateHandler
{
public:
    ScopedTerminateHandler(const std::terminate_handler& handler)
        : m_previous_handler{std::set_terminate(handler)}
    {
    }

    ScopedTerminateHandler(const ScopedTerminateHandler&)            = delete;
    ScopedTerminateHandler& operator=(const ScopedTerminateHandler&) = delete;

    ~ScopedTerminateHandler()
    {
        std::set_terminate(m_previous_handler);
    }

private:
    std::terminate_handler m_previous_handler;
};

} // namespace

ExceptionHandler::ExceptionHandler(std::string context) : m_context(context) {}

bool ExceptionHandler::invoke_void(const std::function<void()>& callable)
{
    // Set the terminate handler
    ScopedTerminateHandler terminate_handler([] {
        LOG.critical("std::terminate was called");
        std::abort();
    });

#if defined(_MSC_VER) && !defined(NDEBUG)
    // In debug mode we turn on memory checking
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    return handle_native_exceptions(m_context, [&] {
        return handle_language_exceptions(m_context, [&] {
            callable();
            return true;
        });
    });
}

} // namespace khepri::application