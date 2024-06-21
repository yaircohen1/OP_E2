#include <iostream>
#include <sstream>
#include <vector>
#include <unistd.h>  // for fork(), execv(), dup2(), getopt 
#include <cstring>   // for strdup()
#include <cstdlib>   // for exit()
#include <sys/wait.h> // for waitpid()
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "mync_server.hpp"
#include "mync_client.hpp"

using namespace std;

// Forward declaration of utility functions
void redirectIO(int inputFd, int outputFd);

// Execute the command with input and output redirection if specified
void executeCommand(const string& command, const string& inputSource, const string& outputDestination);

// Main application entry
int main(int argc, char* argv[]) {
    string command, inputSource, outputDestination;

    // Parsing command-line arguments
    int opt;
    while ((opt = getopt(argc, argv, "e:i:o:b:")) != -1) {
        switch (opt) {
            case 'e':
                command = optarg;
                break;
            case 'i':
                inputSource = optarg;
                break;
            case 'o':
                outputDestination = optarg;
                break;
            case 'b':
                inputSource = outputDestination = optarg;
                break;
            default:
                cerr << "Usage: mync -e <\"command\"> [-i <input source>] [-o <output destination>] [-b <bi-directional source>]" << endl;
                exit(EXIT_FAILURE);
        }
    }

    if (command.empty()) {
        cerr << "Error: Command to execute is required." << endl;
        exit(EXIT_FAILURE);
    }

    // Execute the command
    executeCommand(command, inputSource, outputDestination);
    return 0;
}

// Execute the command with input and output redirection if specified
void executeCommand(const string& command, const string& inputSource, const string& outputDestination) {
    // Setup execution environment
    stringstream commandStream(command);
    string executable;
    vector<string> arguments;
    commandStream >> executable;
    string arg;

    while (commandStream >> arg) {
        arguments.push_back(arg);
    }

    vector<char*> execArgs;
    execArgs.push_back(strdup(executable.c_str())); // Executable name
    for (const string& arg : arguments) {
        execArgs.push_back(strdup(arg.c_str())); // Command arguments
    }
    execArgs.push_back(nullptr); // Null terminate for execv

    // Redirect input/output if specified
    int inputFD = STDIN_FILENO, outputFd = STDOUT_FILENO;
    if (!inputSource.empty()) {
        if (inputSource.substr(0, 4) == "TCPS") {
            int port = stoi(inputSource.substr(4));
            inputFD = startTCPServer(port);
            if (inputFD == -1) {
                cerr << "Failed to start TCP server" << endl;
                exit(EXIT_FAILURE);
            }
            if(inputSource == outputDestination){
                redirectOutput(inputFD);
            }
        }
    }

    if (!outputDestination.empty()){
        if (outputDestination.substr(0, 4) == "TCPC") {
            size_t commaPos = outputDestination.find(',');
            string hostname = outputDestination.substr(4, commaPos - 4);
            int port = stoi(outputDestination.substr(commaPos + 1));
            outputFd = startTCPClient(hostname, port);
            if (outputFd == -1) {
                cerr << "Failed to connect to TCP server" << endl;
                exit(EXIT_FAILURE);
            }
        }
    }
    
    // Fork and execute
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
        redirectIO(inputFD, outputFd);
        execv(execArgs[0], execArgs.data());
        perror("execv failed");
        exit(EXIT_FAILURE);
    } else { // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to complete
        if (WIFEXITED(status)) {
            cout << "Child process exited with status " << WEXITSTATUS(status) << endl;
        } else {
            cout << "Child process did not exit successfully" << endl;
        }
    }

    // Clean up dynamically allocated memory
    for (char* arg : execArgs) {
        free(arg);
    }
}

// Utility function to redirect standard input and output
void redirectIO(int inputFd, int outputFd) {
    if (inputFd != STDIN_FILENO) {
        if (dup2(inputFd, STDIN_FILENO) < 0) {
            perror("Failed to redirect standard input");
            exit(EXIT_FAILURE);
        }
    }

    if (outputFd != STDOUT_FILENO) {
        if (dup2(outputFd, STDOUT_FILENO) < 0) {
            perror("Failed to redirect standard output");
            exit(EXIT_FAILURE);
        }
        if(dup2(outputFd, STDERR_FILENO) <0){
            perror("Failed to redirect standard error output");
            exit(EXIT_FAILURE);
        }
    }
}
