#ifndef UDPSERVER_HPP
#define UDPSERVER_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

int startUDPServer(int port);

#endif