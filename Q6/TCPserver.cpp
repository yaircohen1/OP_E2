#include "TCPserver.hpp"

using namespace std;

int startTCPServer(int port) {
    int listeningSocket = socket(AF_INET, SOCK_STREAM, 0);  // Create a TCP socket for IPv4
    if (listeningSocket < 0){
        perror("Socket creation failed");
        return -1;
    }

    int opt = 1; // Indicates that we want to enable the SO_REUSEADDR and SO_REUSEPORT options.
    // Setting SO_REUSEADDR allows the program to reuse the socket with the same address and port number
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(listeningSocket);
        exit(EXIT_FAILURE);
    }
    
    cout << "Starting TCP server on port " << port << endl;
    struct sockaddr_in address;  // Struct sockaddr_in is defined in the <netinet/in.h> header file.
    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // The server will listen on all available network interfaces.
    address.sin_port = htons(port); // htons() function ensures that the port number is properly formatted for network communication, regardless of the byte order of the host machine.

    // Bind the socket to the specified address and port
    if (bind(listeningSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind() failed");
        close(listeningSocket);
        return -1;
    }

    // Listen for incoming connections
    if (listen(listeningSocket, 1) < 0) {
        perror("listen() failed");
        close(listeningSocket);
        return -1;
    } 

    // Get a connection from the client

    // Define client 
    struct sockaddr_in client_address;
    socklen_t client_address_len;
    memset(&client_address, 0, sizeof(client_address));

    // Accept a connection
    printf("Waiting for TCP connection...\n");
    int clientSocket = accept(listeningSocket, (struct sockaddr *)&client_address, &client_address_len);
    if (clientSocket < 0) {
        perror("Accept failed");
        close(listeningSocket);
        return -1;
    }

    cout << "Accepted connection on port " << port << endl;
    // Close the listening socket as it's no longer needed
    close(listeningSocket);

    return clientSocket;
}

