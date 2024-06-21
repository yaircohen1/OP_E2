#ifndef UDSSERVER_HPP
#define UDSSERVER_HPP

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <string>
#include <vector>
using namespace std;

// create unix domain socket server using stream
int startUDSServerStream(const std::string &socketPath);

// create unix domain socket server using datagram 
int startUDSServerDatagram(const std::string &socketPath);

#endif