#ifndef SERVER_H
#define SERVER_H

#define MAX_SOCKETS 4

UDPsocket *open_socket(int port);
