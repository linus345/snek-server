#ifndef SERVER_H
#define SERVER_H

#include <SDL2/SDL_net.h>
#define MAX_CLIENTS 4

enum Request_Type {
    CLIENT_JOIN = 0,
    SUCCESSFUL_CONNECTION = 1,
    FAILED_CONNECTION = 2,
    UPDATE_SNAKE_POS = 3
};

typedef struct {
    int id;
    IPaddress addr;
} Client;

typedef struct {
    char *host;
    int port;
    UDPsocket udp_sock;
    int nr_of_clients;
    Client clients[MAX_CLIENTS];
} Server;

Server *init_server(char *host, int port);
UDPsocket open_socket(int port);
void log_packet(UDPpacket *pack_recv);
void handle_received_packet(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send);
void handle_join_request(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send);
void handle_update_snake_pos(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send);
void send_connection_success(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send);
void send_connection_failed(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send);
void send_updated_snake_pos(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send);

#endif
