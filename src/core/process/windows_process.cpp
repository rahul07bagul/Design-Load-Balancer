#include "core/process/windows_process.hpp"
#include <stdexcept>

WindowsProcess::WindowsProcess() : process_handle_(nullptr){}

WindowsProcess::~WindowsProcess(){
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

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, NULL, 0);
    if (size_needed <= 0) {
        return false;
    }

    std::wstring wCommand(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, &wCommand[0], size_needed);

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

    if (exit_code == STILL_ACTIVE) {
        return -1;
    }

    return static_cast<int>(exit_code);
}

uint64_t FileTimeToUInt64(const FILETIME &ft) {
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return uli.QuadPart;
}

double WindowsProcess::getCPUUsage(){
    DWORD processID = GetCurrentProcessId();
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (!hProcess) {
        std::cerr << "Failed to open process. Error: " << GetLastError() << std::endl;
        return -1.0;
    }
    
    FILETIME ftCreation, ftExit, ftKernel, ftUser, ftNow;

    if (!GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser)) {
        std::cerr << "GetProcessTimes failed. Error: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return -1.0;
    }
    
    GetSystemTimeAsFileTime(&ftNow);
    uint64_t prevKernel = FileTimeToUInt64(ftKernel);
    uint64_t prevUser   = FileTimeToUInt64(ftUser);
    uint64_t prevTime   = FileTimeToUInt64(ftNow);
    
    Sleep(sampleIntervalMs);
    
    if (!GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser)) {
        std::cerr << "GetProcessTimes failed after sleep. Error: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return -1.0;
    }
    GetSystemTimeAsFileTime(&ftNow);
    
    uint64_t curKernel = FileTimeToUInt64(ftKernel);
    uint64_t curUser   = FileTimeToUInt64(ftUser);
    uint64_t curTime   = FileTimeToUInt64(ftNow);
    
    CloseHandle(hProcess);
    
    uint64_t processTimeDelta = (curKernel - prevKernel) + (curUser - prevUser);
    uint64_t timeDelta = curTime - prevTime;
    
    double cpuUsage = (timeDelta > 0) ? (processTimeDelta * 100.0 / timeDelta) : 0.0;
    
    return cpuUsage;
}

double WindowsProcess::getMemoryUsage(){
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        SIZE_T memUsed = pmc.WorkingSetSize;
        return static_cast<double>(memUsed) / (1024.0 * 1024.0);
    }
}
