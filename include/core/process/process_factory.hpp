#pragma once
#include "process.hpp"
#include <memory>

class ProcessFactory {
public:
    static std::unique_ptr<Process> createProcess();
};