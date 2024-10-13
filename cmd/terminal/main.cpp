#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

// Store gor running processes
std::vector<pid_t> processes;

// SIGINT handler (CTRL+C)
void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", terminating all processes..." << std::endl;
    // Kill all running processes
    for (pid_t pid : processes) {
        kill(pid, SIGKILL);
    }
}

// Launching processes with arguments
void launchProcess(const std::vector<std::string>& commandArgs) {
    // Creating a new process
    pid_t pid = fork();
    if (pid == 0) {
        // In child-process
        std::vector<char*> args;
        for (const auto& arg : commandArgs) {
            // Convert strings to char*
            args.push_back(const_cast<char*>(arg.c_str()));
        }
        // The last argument must be nullptr
        args.push_back(nullptr);
        // Running command
        execvp(args[0], args.data());
        // If execvp returns, error is happened
        std::cerr << "Failed to execute " << commandArgs[0] << ": " << strerror(errno) << std::endl;
        exit(1);
    } else if (pid > 0) {
        // In parent-process
        // Adding PID launched process in array
        processes.push_back(pid);
    } else {
        std::cerr << "Failed to fork: " << strerror(errno) << std::endl;
    }
}

int main() {
    // Set SIGINT handler
    signal(SIGINT, signalHandler);
    std::string input;
    while (true) {
        std::cout << "customshell> ";
        std::getline(std::cin, input);

        // Input command parsing 
        std::istringstream iss(input);
        std::string word;
        std::vector<std::string> commandArgs;
        while (iss >> word) {
            commandArgs.push_back(word);
        }
        if (commandArgs.empty()) {
            continue;
        }
        if (commandArgs[0] == "exit") {
            // Stop the terminal
            std::cout << "Exiting terminal, terminating all processes..." << std::endl;
            signalHandler(SIGINT);
            exit(0);
        } else {
            // Launch the command
            launchProcess(commandArgs);
        }
        // Waiting for processes ending
        int status;
        for (pid_t pid : processes) {
            waitpid(pid, &status, WNOHANG);
        }
    }
    return 0;
}
