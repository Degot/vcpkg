#include "pch.h"
#include "vcpkg_System.h"

namespace vcpkg::System
{
    fs::path get_exe_path_of_current_process()
    {
        wchar_t buf[_MAX_PATH ];
        int bytes = GetModuleFileNameW(nullptr, buf, _MAX_PATH);
        if (bytes == 0)
            std::abort();
        return fs::path(buf, buf + bytes);
    }

    int cmd_execute(const wchar_t* cmd_line)
    {
        // Flush cout before launching external process
        std::cout << std::flush;

        // Basically we are wrapping it in quotes
        const std::wstring& actual_cmd_line = Strings::wformat(LR"###("%s")###", cmd_line);
        int exit_code = _wsystem(actual_cmd_line.c_str());
        return exit_code;
    }

    exit_code_and_output cmd_execute_and_capture_output(const wchar_t* cmd_line)
    {
        // Flush cout before launching external process
        std::cout << std::flush;

        const std::wstring& actual_cmd_line = Strings::wformat(LR"###("%s")###", cmd_line);

        std::string output;
        char buf[1024];
        auto pipe = _wpopen(actual_cmd_line.c_str(), L"r");
        if (pipe == nullptr)
        {
            return { 1, output };
        }
        while (fgets(buf, 1024, pipe))
        {
            output.append(buf);
        }
        if (!feof(pipe))
        {
            return { 1, output };
        }
        auto ec = _pclose(pipe);
        return { ec, output };
    }

    void print(const char* message)
    {
        std::cout << message;
    }

    void println(const char* message)
    {
        print(message);
        std::cout << "\n";
    }

    void print(const color c, const char* message)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo{};
        GetConsoleScreenBufferInfo(hConsole, &consoleScreenBufferInfo);
        auto original_color = consoleScreenBufferInfo.wAttributes;

        SetConsoleTextAttribute(hConsole, static_cast<WORD>(c) | (original_color & 0xF0));
        std::cout << message;
        SetConsoleTextAttribute(hConsole, original_color);
    }

    void println(const color c, const char* message)
    {
        print(c, message);
        std::cout << "\n";
    }

    optional<std::wstring> get_environmental_variable(const wchar_t* varname) noexcept
    {
        wchar_t* buffer;
        _wdupenv_s(&buffer, nullptr, varname);

        if (buffer == nullptr)
        {
            return nullptr;
        }
        std::unique_ptr<wchar_t, void(__cdecl *)(void*)> bufptr(buffer, free);
        return std::make_unique<std::wstring>(buffer);
    }

    void set_environmental_variable(const wchar_t* varname, const wchar_t* varvalue) noexcept
    {
        _wputenv_s(varname, varvalue);
    }

    void Stopwatch2::start()
    {
        static_assert(sizeof(start_time) == sizeof(LARGE_INTEGER), "");

        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&start_time));
    }

    void Stopwatch2::stop()
    {
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&end_time));
        QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&freq));
    }

    double Stopwatch2::microseconds() const
    {
        return (reinterpret_cast<const LARGE_INTEGER*>(&end_time)->QuadPart -
                reinterpret_cast<const LARGE_INTEGER*>(&start_time)->QuadPart) * 1000000.0 / reinterpret_cast<const LARGE_INTEGER*>(&freq)->QuadPart;
    }
}
