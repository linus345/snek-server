#ifndef SERVER_H
#define SERVER_H

#define MAX_CLIENTS 4

UDPsocket open_socket(int port);
void handle_received_packet(UDPpacket *pack_recv);

#endif
