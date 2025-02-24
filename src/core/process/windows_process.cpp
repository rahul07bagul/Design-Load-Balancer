#include "core/process/windows_process.hpp"
#include <stdexcept>

WindowsProcess::WindowsProcess()
    : process_handle_(nullptr)
{
}

WindowsProcess::~WindowsProcess()
{
    if (process_handle_ != nullptr) {
        CloseHandle(process_handle_);
        process_handle_ = nullptr;
    }
}

bool WindowsProcess::start(const std::string& command){
    STARTUPINFOW siw;
    PROCESS_INFORMATION pi;

    ZeroMemory(&siw, sizeof(siw));
    siw.cb = sizeof(siw);
    ZeroMemory(&pi, sizeof(pi));

    // Convert the command to a wide string
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, NULL, 0);
    if (size_needed <= 0) {
        return false; // Could not convert string
    }

    std::wstring wCommand(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, &wCommand[0], size_needed);

    // CreateProcessW expects the command line in wCommand
    if (!CreateProcessW(
            nullptr,         // No module name (use command line)
            &wCommand[0],    // Command line (in/out)
            nullptr,         // Process handle not inheritable
            nullptr,         // Thread handle not inheritable
            FALSE,           // Set handle inheritance to FALSE
            0,               // Creation flags
            nullptr,         // Use parent's environment block
            nullptr,         // Use parent's working directory
            &siw,            // Pointer to STARTUPINFOW
            &pi))            // Pointer to PROCESS_INFORMATION
    {
        return false;
    }

    CloseHandle(pi.hThread);

    process_handle_ = pi.hProcess;
    return true;
}

bool WindowsProcess::isRunning(){
    if (process_handle_ == nullptr) {
        return false;
    }

    DWORD exit_code;
    if (!GetExitCodeProcess(process_handle_, &exit_code)) {
        return false;
    }

    return (exit_code == STILL_ACTIVE);
}

void WindowsProcess::terminate(){
    if (process_handle_ != nullptr) {
        TerminateProcess(process_handle_, 0);
    }
}

int WindowsProcess::getExitCode(){
    if (process_handle_ == nullptr) {
        return -1;
    }

    DWORD exit_code;
    if (!GetExitCodeProcess(process_handle_, &exit_code)) {
        return -1;
    }

    // If STILL_ACTIVE, the process is running, so return -1 or some sentinel
    if (exit_code == STILL_ACTIVE) {
        return -1;
    }

    return static_cast<int>(exit_code);
}
