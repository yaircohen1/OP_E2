#ifndef UDSCLIENT_HPP
#define UDSCLIENT_HPP
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <string>

using namespace std;

int startUDSClientStream(const string &socketPath); 

int startUDSClientDatagram(const string &socketPath);

#endif