#pragma once
#include "core/process/process.hpp"
#include <windows.h>
#include <psapi.h>

class WindowsProcess : public Process {
public:
    WindowsProcess();
    ~WindowsProcess();
    bool start(const std::string& command) override;
    bool isRunning() override;
    void terminate() override;
    int getExitCode() override;
    double getCPUUsage() override;
    double getMemoryUsage() override;
private:
    HANDLE process_handle_;
    DWORD sampleIntervalMs = 1000;
};