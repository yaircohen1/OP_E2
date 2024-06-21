
#ifndef TCPCLIENT_HPP
#define TCPCLIENT_HPP

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

using namespace std;

int startTCPClient(const string &hostname, int port);

#endif
