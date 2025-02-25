#include "core/process/linux_process.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

LinuxProcess::LinuxProcess()
    : pid_(-1){}

LinuxProcess::~LinuxProcess(){
    if (pid_ > 0) {
        kill(pid_, SIGTERM);
    }
    int status = 0;
    waitpid(pid_, &status, 0);
    pid_ = -1;
}

bool LinuxProcess::start(const std::string& command){
    pid_ = fork();
    if (pid_ < 0) {
        return false;
    }

    if (pid_ == 0) {
        execl("/bin/sh", "sh", "-c", command.c_str(), (char*)nullptr);
        _exit(127);
    }

    return true;
}

bool LinuxProcess::isRunning(){
    if (pid_ <= 0) {
        return false;
    }

    int status = 0;
    pid_t result = waitpid(pid_, &status, WNOHANG);
    if (result == 0) {
        return true;
    } else if (result == pid_) {
        return false;
    } else {
        return false;
    }
}

void LinuxProcess::terminate(){
    if (pid_ > 0) {
        kill(pid_, SIGTERM);
    }
}

int LinuxProcess::getExitCode(){
    if (pid_ <= 0) {
        return -1;
    }

    int status = 0;
    pid_t result = waitpid(pid_, &status, WNOHANG);
    if (result == pid_) {
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    }
    return -1;
}

double LinuxProcess::getCPUUsage(){
    return 0.0;
}

double LinuxProcess::getMemoryUsage(){
    return 0.0;
}
