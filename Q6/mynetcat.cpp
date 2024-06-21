#include "TCPserver.hpp"
#include "TCPclient.hpp"
#include "UDPserver.hpp"
#include "UDPclient.hpp"
#include "UDSclient.hpp"
#include "UDSserver.hpp"

#include <iostream>
#include <vector>
#include <cstring> // for strdup()
#include <unistd.h> // for fork(), execv(), dup2(), getopt
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <csignal>
#include <sys/wait.h> // for waitpid()
#include <ctime>
#include <signal.h>

#define BUFFER_SIZE 1024


using namespace std;


// Redirect input/output if specified
int inputFd = -1, outputFd = -1;
int serverSocket = -1, clientSocket = -1; // Server and client sockets
pid_t pid = -1;
bool eFlag = false;
vector<char*> execArgs;

void closeProgram (int signal,vector<char*>& execArgs){
    if (inputFd != -1 && inputFd != STDIN_FILENO) {
        close(inputFd);
    }
    if (outputFd != -1 && outputFd != STDOUT_FILENO && outputFd != inputFd) {
        close(outputFd);
    }
    if (serverSocket != -1) {
        close(serverSocket);
    }
    if (clientSocket != -1) {
        close(clientSocket);
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

void closeProgramT (int signal) {
    closeProgram (signal,execArgs);
}


// Print to the stdout from the inputFd and send to outpudFd from the stdin
int chat(int inputFd, int outputFd);


// Execute the command with input and output redirection if specified
void executeCommand(const string& command, const string& inputSource, const string& outputDestination);

// Main application entry
int main(int argc, char* argv[]) { 
    string command, inputSource, outputDestination ,timeout;

    // Parsing command-line arguments
    int opt;
    while ((opt = getopt(argc, argv, "e:i:o:b:t:")) != -1) {
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
            case 't':
                timeout = optarg;
                break;
            default:
                cerr << "Usage: mync -e <\"command\"> [-i <input source>] [-o <output destination>] [-b <bi-directional source>] [-t <timeout>]" << endl;
                exit(EXIT_FAILURE);
        }
    }
    if (!timeout.empty()) {
        signal(SIGALRM, closeProgramT);
        int time = stoi(timeout);
        alarm(time);
    }

    // Execute the command
    executeCommand(command, inputSource, outputDestination);
    return 0;
}

// Execute the command with input and output redirection if specified
void executeCommand(const string& command, const string& inputSource, const string& outputDestination) {
    // Setup execution environment
    stringstream commandStream(command); //create string stream
    string executable; // store file name that run
    vector<string> arguments;
    commandStream >> executable; 
    string arg;
    // loop itreate all the args from the stream and add it to arguments vector
    while (commandStream >> arg) {
        arguments.push_back(arg);
    }

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
            if(dup2 (inputFd, STDIN_FILENO) < 0) {
                perror("Failed to redirect standard input");
                closeProgram(EXIT_FAILURE, execArgs);
            }
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
        } else if (inputSource.substr(0, 4) == "UDPS") {
            int port = stoi(inputSource.substr(4));
            inputFd = startUDPServer(port); // Return the serverSocket from UDP server
            if (inputFd == -1) {
                perror("Failed to start UDP server");
                closeProgram(EXIT_FAILURE, execArgs);
            }
            if(dup2 (inputFd, STDIN_FILENO) < 0) {
                perror("Failed to redirect standard input");
                closeProgram(EXIT_FAILURE, execArgs);
            }
            close(inputFd); // close the serverSocket from UDP server
        } else if (inputSource.substr(0, 5) == "UDSSS") {  // Unix-Domain-Socket-Stream-Server: redirection input from the client (-i Flag)
            int findPath = inputSource.find("/");
            const string socketPath = inputSource.substr(findPath);
            serverSocket = startUDSServerStream(socketPath); // we get the listener socket
            if(serverSocket == -1) {
                perror("Failed to start UDS-Server-Stream");
                closeProgram(EXIT_FAILURE, execArgs);
            }
        
            struct sockaddr_un client_address; // Define client 
            socklen_t client_address_len; // Define the client_address_len
            
            // Accept a connection from the client
            inputFd = accept(serverSocket, (struct sockaddr *)&client_address, &client_address_len);
            if (inputFd == -1) {
                perror("Accept failed");
                close (serverSocket);
                closeProgram(EXIT_FAILURE, execArgs);
            }
            cout<<"Connection accepted from client"<<endl;
            if (dup2(inputFd, STDIN_FILENO) < 0) {
                perror("Failed to redirect standard input");
                closeProgram(EXIT_FAILURE, execArgs);
            }

        } else if (inputSource.substr(0, 5) == "UDSSD") { //  // Unix-Domain-Socket-Datagram-Server: redirection input from the client (-i Flag)
            int findPath = inputSource.find("/");
            const string socketPath = inputSource.substr(findPath);
            inputFd = startUDSServerDatagram(socketPath);
            if(inputFd == -1) {
                cerr << "Failed to start UDS-Server-Datagram" << endl;
                closeProgram(EXIT_FAILURE, execArgs);
            }
            if (dup2(inputFd, STDIN_FILENO) < 0) {
                perror("Failed to redirect standard input");
                closeProgram(EXIT_FAILURE, execArgs);
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
        } else if (outputDestination.substr(0, 4) == "UDPC") {
            size_t commaPos = outputDestination.find(',');
            string hostname = outputDestination.substr(4, commaPos - 4);
            int port = stoi(outputDestination.substr(commaPos + 1));
            outputFd = startUDPClient(hostname, port); // Return client socket from UDP client
            if (outputFd == -1) {
               perror("Failed to connect to UDP client");
               closeProgram(EXIT_FAILURE, execArgs);
            }
            if (dup2(outputFd, STDOUT_FILENO) < 0) {
                perror("Failed to redirect standard output");
                closeProgram(EXIT_FAILURE, execArgs);
            }
            if(dup2(outputFd, STDERR_FILENO) < 0) {
                perror("Failed to redirect standard error output");
                closeProgram(EXIT_FAILURE, execArgs);
            }      
        } else if (outputDestination.substr(0, 5) == "UDSCS") { /* Unix-Domain-Socket-Stream-Client: redirection output to the client (-o Flag) */
            size_t find_path = outputDestination.find("/");
            const string socketPath = outputDestination.substr(find_path);
            clientSocket = startUDSClientStream(socketPath);
            if (clientSocket == -1) {
                cerr << "Failed to connect to UDS-Client-Stream server" << endl;
                closeProgram(EXIT_FAILURE, execArgs);
            }
            if (dup2(clientSocket, STDOUT_FILENO) < 0) {
                perror("Failed to redirect standard output");
                closeProgram(EXIT_FAILURE, execArgs);
            }
            if(dup2(clientSocket, STDERR_FILENO) < 0) {
                perror("Failed to redirect standard error output");
                closeProgram(EXIT_FAILURE, execArgs);
            }

        } else if (outputDestination.substr(0, 5) == "UDSCD") {  /* Unix-Domain-Socket-Datagram-Client: redirection output to the client (-o Flag) */
            size_t find_path = outputDestination.find("/");
            const string socketPath = outputDestination.substr(find_path);
            outputFd = startUDSClientDatagram(socketPath);
            if (outputFd == -1) {
            cerr << "Failed to connect to UDS-Client-Datagram server" << endl;
            closeProgram(EXIT_FAILURE, execArgs);
            }
            if (dup2(outputFd, STDOUT_FILENO) < 0) {
                perror("Failed to redirect standard output");
                closeProgram(EXIT_FAILURE, execArgs);
            }
            if(dup2(outputFd, STDERR_FILENO) < 0) {
                perror("Failed to redirect standard error output");
                closeProgram(EXIT_FAILURE, execArgs);
            }
        }
    }

    if (eFlag) {
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
    }  else { // No flag -e
        cout<< "print 'Quit' if want to leave"<<endl;
        int status = chat(inputFd, outputFd);
        if (status!= 0 && status!=1) {
            cout << "Process did not exit successfully" << endl;
        }
        closeProgram(EXIT_FAILURE, execArgs);
        if (status == 1) { // User want to quit
            closeProgram(EXIT_FAILURE, execArgs);
        } 
    }
}

int chat(int inputFd, int outputFd) {
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
        if (select(maxFd + 1, &readFds, NULL, NULL, NULL) == -1) {
            perror("select Failed");
            return -1;
        }

        // check if the inputFd has data to read (from the input source)
        if (inputFd != STDIN_FILENO && FD_ISSET(inputFd, &readFds)) {
            char buffer[1024];
            int bytes_read = read(inputFd, buffer, sizeof(buffer)-1);  // read from the inputFd
            if (bytes_read == -1) {
                perror("read Failed");
                return -1;
            }
            if (bytes_read == 0) {
                break;
            }
            buffer[bytes_read] = '\0';  // Null-terminate the buffer
            if (strcmp(buffer, "Quit\n") == 0 || strcmp(buffer, "Quit") == 0) {
                return 1;
            }
            // write to the stdout
            if (write(STDOUT_FILENO, buffer, bytes_read) == -1) {
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
