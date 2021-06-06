#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include <SDL2/SDL_net.h>
#define MAX_CLIENTS 4

enum Request_Type {
    CLIENT_JOIN = 0,
    NEW_CLIENT_JOINED = 1,
    SUCCESSFUL_CONNECTION = 2,
    FAILED_CONNECTION = 3,
    UPDATE_SNAKE_POS = 4,
    RANDOM_POS = 5,
    SEND_TICKS = 6,
    COLLISION = 7,
    ATE_FRUIT = 8,
    COLOR_CHANGE = 9
};

typedef struct {
    bool started;
    int nr_of_fruits;
    int fruits[MAX_CLIENTS]; // used to see which index is free to use
} Game_State;

typedef struct {
    int id;
    bool alive;
    bool ready_to_start;
    IPaddress addr;
} Client;

typedef struct {
    char *host;
    int port;
    UDPsocket udp_sock;
    int nr_of_clients;
    Client clients[MAX_CLIENTS];
    Game_State game_state;
} Server;

/* typedef struct { */
/*     Server *server; */
/*     UDPpacket *pack_recv; */
/*     UDPpacket *pack_send; */
/* } Thread_Args; */

Server *init_server(char *host, int port);
UDPsocket open_socket(int port);
void log_packet(UDPpacket *pack_recv);
/* void listen_for_packets(Server *server, UDPpacket *pack_recv); */
void handle_received_packet(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send, unsigned ticks);
void handle_join_request(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send, unsigned ticks);
void handle_update_snake_pos(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send);
void handle_color_change(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send);
void handle_collision(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send);
void handle_ate_fruit(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send);
void send_connection_success(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send, unsigned ticks);
void send_connection_failed(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send);
void send_updated_snake_pos(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send);
void send_color_change(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send);
void send_random_fruit_pos(Server *server, UDPpacket *pack_send);
void send_ticks(Server *server, UDPpacket *pack_send);

#endif
