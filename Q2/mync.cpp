#include <iostream>
#include <sstream>
#include <vector>
#include <unistd.h>  // for fork() and execv()
#include <cstring>   // for strdup()
#include <cstdlib>   // for exit() and free()
#include <sys/wait.h> // for waitpid()

using namespace std;


int main(int argc, char* argv[]) {
    // Ensure correct usage
    if (argc != 3 || string(argv[1]) != "-e") {
        cerr << "Invalid Arguments" << endl;
        exit(EXIT_FAILURE);
    }

    // Extract the full command from the command line argument
    string fullCommand = argv[2];
    stringstream commandStream(fullCommand);

    // Parse the command to extract the executable name and its arguments
    string executableName;
    commandStream >> executableName; // First part is the executable name

    vector<string> argumentList;
    string argument;
    while (commandStream >> argument) {
        argumentList.push_back(argument);
    }

    // Prepare the arguments for execv, which requires a null-terminated array of char*
    vector<char*> execArguments;
    execArguments.push_back(strdup(executableName.c_str())); // Add the executable name
    for (const string& arg : argumentList) {
        execArguments.push_back(strdup(arg.c_str())); // Add each argument
    }
    execArguments.push_back(nullptr); // Null-terminate added 
    // Fork the process to create a child process
    pid_t processID = fork();

    if (processID < 0) {
        perror("Failed to fork");
        return EXIT_FAILURE;
    } else if (processID == 0) {
        // Child process 
        
        // Redirect stdin, stdout, and stderr to the same as the parent process
        if (dup2(STDIN_FILENO, STDIN_FILENO) == -1 ||
            dup2(STDOUT_FILENO, STDOUT_FILENO) == -1 ||
            dup2(STDERR_FILENO, STDERR_FILENO) == -1) {
            perror("Failed to redirect standard streams");
            exit(EXIT_FAILURE);
        }
        
        // Execute the command
        execv(execArguments[0], execArguments.data());
        // If execv returns, it has failed
        perror("Failed to execute command");
        exit(EXIT_FAILURE);
    } else {
        // Parent process: wait for the child to finish
        int status;
        waitpid(processID, &status, 0);
        if (WIFEXITED(status)) {
            cout << "Child process exited with status " << WEXITSTATUS(status) << endl;
        } else {
            cout << "Child process execution failed" << endl;
        }
    }

    // Free the memory allocated for the arguments
    for (char* arg : execArguments) {
        free(arg);
    }

    return EXIT_SUCCESS;
}
