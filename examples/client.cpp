#include <windows.h>
#include <iostream>

// Helper to convert FILETIME to a 64-bit integer (in 100-nanosecond units)
uint64_t FileTimeToUInt64(const FILETIME &ft) {
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return uli.QuadPart;
}

// This function returns the CPU usage percentage for a process over a sample interval (default 1000ms).
// Note: This percentage is relative to a single CPU core.
double GetProcessCpuUsage(DWORD processID, DWORD sampleIntervalMs = 1000) {
    // Open the target process (adjust access rights as necessary)
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (!hProcess) {
        std::cerr << "Failed to open process. Error: " << GetLastError() << std::endl;
        return -1.0;
    }
    
    FILETIME ftCreation, ftExit, ftKernel, ftUser, ftNow;
    // Get initial process times
    if (!GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser)) {
        std::cerr << "GetProcessTimes failed. Error: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return -1.0;
    }
    
    // Get current system time
    GetSystemTimeAsFileTime(&ftNow);
    uint64_t prevKernel = FileTimeToUInt64(ftKernel);
    uint64_t prevUser   = FileTimeToUInt64(ftUser);
    uint64_t prevTime   = FileTimeToUInt64(ftNow);
    
    // Wait for the sampling interval
    Sleep(sampleIntervalMs);
    
    // Get process times again
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
    
    // Calculate the deltas for the process CPU time and the elapsed wall time
    uint64_t processTimeDelta = (curKernel - prevKernel) + (curUser - prevUser);
    uint64_t timeDelta = curTime - prevTime;
    
    // Compute the CPU usage as a percentage (relative to one core)
    double cpuUsage = (timeDelta > 0) ? (processTimeDelta * 100.0 / timeDelta) : 0.0;
    
    return cpuUsage;
}

int main() {
    // For example, measure CPU usage for the current process
    std::string temp = "rahuk";
    for(int i = 0; i < 1000000; i++) {
        // Do some work
        temp = temp + "rahuk";
    }
    DWORD processID = GetCurrentProcessId();
    std::cout << "Measuring CPU usage for process ID: " << processID << std::endl;
    
    double usage = GetProcessCpuUsage(processID);
    if (usage < 0) {
        std::cerr << "Error measuring CPU usage." << std::endl;
    } else {
        std::cout << "CPU Usage: " << usage << "%" << std::endl;
    }
    
    return 0;
}
