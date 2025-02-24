#pragma once
#include "core/process/process.hpp"
#include <windows.h>

class WindowsProcess : public Process {
public:
    WindowsProcess();
    ~WindowsProcess();
    bool start(const std::string& command) override;
    bool isRunning() override;
    void terminate() override;
    int getExitCode() override;
private:
    HANDLE process_handle_;
};