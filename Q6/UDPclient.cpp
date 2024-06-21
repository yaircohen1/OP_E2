#include "UDPclient.hpp"


using namespace std;

// Function to start a UDP client
int startUDPClient(const string &hostname, int port) {
    // addressCriteria - a structure that provides criteria for selecting IP addresses.
    struct addrinfo addressCriteria, *IPlist, *Iterator;
    int clientSocket; 

    // Set up the addressCriteria structure
    memset(&addressCriteria, 0, sizeof(addressCriteria));
    addressCriteria.ai_socktype = SOCK_DGRAM; // Set the socket type to UDP

    // getaddrinfo is suitable foR IPv4 & IPv6 to Get IP address list from the server
    int status;
    string port_str = to_string(port);
    if ((status = getaddrinfo(hostname.c_str(), port_str.c_str(), &addressCriteria, &IPlist)) != 0) {
        cerr << "getaddrinfo: " << gai_strerror(status) << endl;
        return -1;
    }

    // connect to the first IP from the list to match
    for (Iterator = IPlist; Iterator != NULL; Iterator = Iterator->ai_next) {
        if ((clientSocket = socket(Iterator->ai_family, Iterator->ai_socktype, Iterator->ai_protocol)) == -1) {
            perror("Error creating socket");
            continue;
        }
        
        // Connect to the server

        // When using connect with a UDP socket, it's not creating a connection as with TCP. 
        // Instead, connect in UDP defines the destination address of the socket 
        // so that it does not need to be specified in every call to sendto.
        
        if (connect(clientSocket, Iterator->ai_addr, Iterator->ai_addrlen) == -1) {
            close(clientSocket);
            perror("Error connecting to server");
            continue;
        }

        break; // If we get here, we must have connected successfully
    }

    // Check if we failed to connect to any address
    if (Iterator == NULL) {
        cerr << "Failed to connect" << endl;
        return -1;
    }

    // Free the linked list
    freeaddrinfo(IPlist);

    return clientSocket;
}