#include "core/process/process_factory.hpp"

#ifdef _WIN32
    #include "core/process/windows_process.hpp"
#else
    #include "core/process/linux_process.hpp"
#endif

std::unique_ptr<Process> ProcessFactory::createProcess() {
    #ifdef _WIN32
        return std::make_unique<WindowsProcess>();
    #else
        return std::make_unique<LinuxProcess>();
    #endif
}

