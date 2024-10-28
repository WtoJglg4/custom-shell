#include <iostream>
#include <vector>
#include <string>
#include <signal.h>
#include <sys/wait.h>
#include <filesystem>
#include <fstream>
#include <unistd.h>
#include <sys/resource.h>
#include <cstring>
#include <sstream>

namespace fs = std::__fs::filesystem;

std::vector<pid_t> processes;

void signal_handler(int signal) {
    std::cout << "\nЗавершаем все процессы..." << std::endl;
    for (pid_t pid : processes) {
        kill(pid, SIGKILL);
    }
    processes.clear();
    exit(0);
}

void ls_command() {
    for (const auto &entry : fs::directory_iterator(".")) {
        std::cout << entry.path().filename().string() << "\n";
    }
}

void cat_command(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл: " << filename << std::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << std::endl;
    }
}

void nice_command(int priority, const std::vector<std::string>& args) {
    if (args.empty()) {
        if (setpriority(PRIO_PROCESS, 0, priority) == -1) {
            std::cerr << "Не удалось изменить приоритет текущего процесса" << std::endl;
        } else {
            std::cout << "Приоритет текущего процесса изменен на " << priority << std::endl;
        }
    } else {
        pid_t pid = fork();
        if (pid == 0) {
            setpriority(PRIO_PROCESS, 0, priority);

            std::vector<char*> exec_args;
            for (const auto& arg : args) {
                exec_args.push_back(const_cast<char*>(arg.c_str()));
            }
            exec_args.push_back(nullptr);

            execvp(exec_args[0], exec_args.data());
            std::cerr << "Ошибка запуска команды: " << args[0] << std::endl;
            exit(1);
        } else if (pid > 0) {
            processes.push_back(pid);
        } else {
            std::cerr << "Ошибка создания процесса" << std::endl;
        }
    }
}

void killall_command(const std::string &process_name) {
    std::string command = "killall " + process_name;
    int result = system(command.c_str());
    
    if (result == -1) {
        std::cerr << "Ошибка при выполнении killall для процесса: " << process_name << std::endl;
    } else {
        std::cout << "Все процессы с именем " << process_name << " завершены." << std::endl;
    }
}

enum CommandType { LS, CAT, NICE, KILLALL, UNKNOWN };

void execute_command(const std::string &input) {
    std::istringstream iss(input);
    std::vector<std::string> tokens;
    for (std::string s; iss >> s; )
        tokens.push_back(s);

    if (tokens.empty()) return;

    CommandType commandType;
    if (tokens[0] == "ls") commandType = LS;
    else if (tokens[0] == "cat") commandType = CAT;
    else if (tokens[0] == "nice") commandType = NICE;
    else if (tokens[0] == "killall") commandType = KILLALL;
    else commandType = UNKNOWN;

    switch (commandType) {
        case LS:
            ls_command();
            break;

        case CAT:
            if (tokens.size() < 2) {
                std::cerr << "Укажите имя файла для cat" << std::endl;
            } else {
                cat_command(tokens[1]);
            }
            break;

        case NICE:
            if (tokens.size() < 2) {
                std::cerr << "Укажите значение приоритета для nice" << std::endl;
            } else {
                int priority = std::stoi(tokens[1]);
                std::vector<std::string> args(tokens.begin() + 2, tokens.end());
                nice_command(priority, args);
            }
            break;

        case KILLALL:
            if (tokens.size() < 2) {
                std::cerr << "Укажите имя процесса для killall" << std::endl;
            } else {
                killall_command(tokens[1]);
            }
            break;

        default:
            std::cout << "Команда не поддерживается терминалом: " << tokens[0] << std::endl;
            break;
    }
}

int main() {

    signal(SIGINT, signal_handler);

    std::string input;
    std::cout << "Добро пожаловать в собственный терминал! (нажмите CTRL+C для выхода)\n";

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "exit") {
            signal_handler(SIGINT);
        } else {
            execute_command(input);
        }
    }

    return 0;
}
