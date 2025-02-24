#pragma once
#include "core/process/process.hpp"

class LinuxProcess : public Process {
public:
    LinuxProcess();
    ~LinuxProcess();
    bool start(const std::string& command) override;
    bool isRunning() override;
    void terminate() override;
    int getExitCode() override;
private:
    pid_t pid_;
};