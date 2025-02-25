#pragma once
#include <string>
#include <iostream>

class Process {
public:
    virtual ~Process() = default;
    virtual bool start(const std::string& command) = 0;
    virtual bool isRunning() = 0;
    virtual void terminate() = 0;
    virtual int getExitCode() = 0;
    virtual double getCPUUsage() = 0;
    virtual double getMemoryUsage() = 0;
};