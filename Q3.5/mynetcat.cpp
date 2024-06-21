#include "TCPserver.hpp"
#include "TCPclient.hpp"


#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <csignal>
#include <sys/wait.h>
#include <ctime>
#include <signal.h>

#define BUFFER_SIZE 1024


using namespace std;


// Redirect input/output if specified
int inputFd = -1, outputFd = -1;
pid_t pid = -1;
bool eFlag = false;

void closeProgram (int signal,vector<char*>& execArgs){
    if (inputFd != -1 && inputFd != STDIN_FILENO) {
        close(inputFd);
    }
    if (outputFd != -1 && outputFd != STDOUT_FILENO && outputFd != inputFd) {
        close(outputFd);
    }
    if (pid != -1) {
        kill(pid, SIGTERM);
        cout << "Killed child process." << endl;
        waitpid(pid, nullptr, 0); // Wait for child process to terminate
    }
    for (char* arg : execArgs) {
                free(arg);
            }
            fflush(stdout);
    exit(signal);
}

//Print to the stdout from the inputFd and send to outpudFd from the stdin
int chat(int inputFd, int outputFd);

// Execute the command with input and output redirection if specified
void executeCommand(const string& command, const string& inputSource, const string& outputDestination);

// Main application entry
int main(int argc, char* argv[]) { 
    string command, inputSource, outputDestination ,timeout;

    // Parsing command-line arguments
    int opt;
    while ((opt = getopt(argc, argv, "e:i:o:b:")) != -1) {
        switch (opt) {
            case 'e':
                command = optarg;
                eFlag = true;
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

    
    if (!inputSource.empty()) {
        if (inputSource.substr(0, 4) == "TCPS") {
            int port = stoi(inputSource.substr(4));
            inputFd = startTCPServer(port); // Got the clientSocket from TCP server
            if (inputFd == -1) {
                perror("Failed to start TCP server");
                closeProgram(EXIT_FAILURE, execArgs);
            }
            dup2 (inputFd, STDIN_FILENO);
            if(inputSource == outputDestination){
                if(dup2(inputFd, STDOUT_FILENO) < 0) {
                    perror("Failed to redirect standard output");
                   closeProgram(EXIT_FAILURE, execArgs);
                }
                if(dup2(inputFd, STDERR_FILENO)< 0){
                    perror("Failed to redirect standard error output");
                    closeProgram(EXIT_FAILURE, execArgs);
                }
            }
        } 
    }
     
    if (!outputDestination.empty()){
        if (outputDestination.substr(0, 4) == "TCPC") {
            size_t commaPos = outputDestination.find(',');
            string hostname = outputDestination.substr(4, commaPos - 4);
            int port = stoi(outputDestination.substr(commaPos + 1));
            outputFd = startTCPClient(hostname, port); // Return socketClient from TCP client
            if (outputFd == -1) {
                perror("Failed to connect to TCP client");
                closeProgram(EXIT_FAILURE, execArgs);
            }
            if(dup2(outputFd, STDOUT_FILENO) < 0) {
                perror("Failed to redirect standard output");
                closeProgram(EXIT_FAILURE, execArgs);
            }
            if(dup2(outputFd, STDERR_FILENO) < 0) {
                perror("Failed to redirect standard error output");
                closeProgram(EXIT_FAILURE, execArgs);
            }
        }  
    }

    if (eFlag){
        // Fork and execute
        pid = fork();
        if (pid == -1) {
            perror("fork");
            closeProgram(EXIT_FAILURE, execArgs);
        }

        if (pid == 0) { // Child process
            execv(execArgs[0], execArgs.data());
            perror("execv failed");
            closeProgram(EXIT_FAILURE, execArgs);

        } else { // Parent process
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                    cout << "Child process exited with status " << WEXITSTATUS(status) << endl;
                } else {
                    cout << "Child process did not exit successfully" << endl;
            }
            closeProgram(EXIT_FAILURE, execArgs);
        }   
    } else { // No flag -e
        cout<< "print 'Quit' if want to leave"<<endl;
        int status = chat(inputFd, outputFd);
        if (status!= 0 && status!=1) {
            cout << "Process did not exit successfully" << endl;
        }
        closeProgram(EXIT_FAILURE, execArgs);
        if (status ==1) { // User want to quit
            closeProgram(EXIT_FAILURE, execArgs);
        } 
    }
}

int chat(int inputFd, int outputFd)
{
    fd_set readFds;
    int maxFd = inputFd;
    // if the outputFd is greater than the inputFd, set the maxFd to the outputFd
    if (outputFd > maxFd) {
        maxFd = outputFd;
    }
    if (STDIN_FILENO > maxFd) {
        maxFd = STDIN_FILENO;
    }
    
    while (1) 
    {
        FD_ZERO(&readFds); // clear the readFds set
        FD_SET(inputFd, &readFds);  // add the inputFd to the readFds set
        FD_SET(STDIN_FILENO, &readFds); // add the stdin to the readFds set

        // wait for any of the file descriptors to have data to read
        if (select(maxFd + 1, &readFds, NULL, NULL, NULL) == -1) 
        {
            perror("select Failed");
            return -1;
        }

        // check if the inputFd has data to read (from the input source)
        if (inputFd != STDIN_FILENO && FD_ISSET(inputFd, &readFds)) 
        {
            char buffer[1024];
            int bytes_read = read(inputFd, buffer, sizeof(buffer)-1);  // read from the inputFd
            if (bytes_read == -1) 
            {
                perror("read Failed");
                return -1;
            }
            if (bytes_read == 0) 
            {
                break;
            }
            buffer[bytes_read] = '\0';  // Null-terminate the buffer
            if (strcmp(buffer, "Quit\n") == 0 || strcmp(buffer, "Quit") == 0) {
                return 1;
            }
            // write to the stdout
            if (write(STDOUT_FILENO, buffer, bytes_read) == -1) 
            {
                perror("write Failed 1");
                return -1;
            }
        }

        // check if the stdin has data to read (keyboard input)
        if (FD_ISSET(STDIN_FILENO, &readFds) && outputFd != STDOUT_FILENO) 
        {
            char buffer[1024];
            int bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer)-1);  // read from the stdin
            if (bytes_read == -1) 
            {
                perror("read Failed");
                return -1;
            }
            if (bytes_read == 0) 
            {
                break;
            }
            buffer[bytes_read] = '\0';  // Null-terminate the buffer
            if (strcmp(buffer, "Quit\n") == 0 || strcmp(buffer, "Quit") == 0) {
                return 1;
            }
            if (write(STDOUT_FILENO, buffer, bytes_read) == -1) 
            {
                perror("write Failed 2");
                return -1;
            }
        }
    }

    return 0;
}



