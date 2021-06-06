#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_net.h>
#include "server.h"

Server *init_server(char *host, int port)
{
    Server *server = malloc(sizeof(Server));

    server->host = host;
    server->port = port;
    server->nr_of_clients = 0;
    server->game_state.started = false;
    server->game_state.nr_of_fruits = 0;
    for(int i = 0; i < MAX_CLIENTS; i++) {
        server->game_state.fruits[i] = 0;
    }

    return server;
}

UDPsocket open_socket(int port)
{
    UDPsocket udp_sock = SDLNet_UDP_Open(port);

    if(!udp_sock) {
        printf("SDLNet_UDP_Open: %s", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    return udp_sock;
}

void log_packet(UDPpacket *pack_recv)
{
    printf("Channel: %d\n", pack_recv->channel);
    printf("Data: %s\n", pack_recv->data);
    printf("Len: %d\n", pack_recv->len);
    printf("Max len: %d\n", pack_recv->maxlen);
    printf("Status: %d\n", pack_recv->status);
    printf("src host: %u\n", pack_recv->address.host);
    printf("src port: %u\n", pack_recv->address.port);
}

/* void listen_for_packets(Server *server, UDPpacket *pack_recv) */
/* { */
/* } */

void handle_received_packet(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send, unsigned ticks)
{
    int type;
    // get request type from packet
    sscanf(pack_recv->data, "%d", &type);

    switch(type) {
        case CLIENT_JOIN:
            handle_join_request(server, pack_recv, pack_send, ticks);
            break;
        case UPDATE_SNAKE_POS:
            handle_update_snake_pos(server, pack_recv, pack_send);
            break;
        case COLLISION:
            handle_collision(server, pack_recv, pack_send);
            break;
        case ATE_FRUIT:
            handle_ate_fruit(server, pack_recv, pack_send);
            break;
        case COLOR_CHANGE:
            handle_color_change(server, pack_recv, pack_send);
            break;
        case START_GAME:
            handle_start_game(server, pack_recv, pack_send);
            break;
    }
}

/* void handle_received_packet(void *args) */
/* { */
/*     Thread_Args *a = (Thread_Args *) args; */

/*     int type; */
/*     // get request type from packet */
/*     sscanf(a->pack_recv->data, "%d", &type); */

/*     switch(type) { */
/*         case CLIENT_JOIN: */
/*             handle_join_request(a->server, a->pack_recv, a->pack_send); */
/*             break; */
/*         case UPDATE_SNAKE_POS: */
/*             handle_update_snake_pos(a->server, a->pack_recv, a->pack_send); */
/*             break; */
/*     } */
/* } */

void handle_join_request(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send, unsigned ticks)
{
    if(server->nr_of_clients < MAX_CLIENTS) {
        // client successfully joined server
        // give client id
        server->clients[server->nr_of_clients].id = server->nr_of_clients;
        server->clients[server->nr_of_clients].alive = true;
        server->clients[server->nr_of_clients].ready_to_start = false;
        // add client ip to connected clients, increment of nr_of_clients is done in send_connection_success
        server->clients[server->nr_of_clients].addr = pack_recv->address;
        printf("client connected\n");
        // send back connection success
        send_connection_success(server, pack_recv, pack_send, ticks);
    } else {
        // already 4 clients connected so connection failed
        // send back connection failure
        send_connection_failed(server, pack_recv, pack_send);
    }
}

void handle_start_game(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send)
{
    server->game_state.started = true;

    send_start_game(server, pack_send);
}

void handle_update_snake_pos(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send)
{
    int id, temp;
    // get client id
    sscanf(pack_recv->data, "%d %d", &temp, &id);
    for(int i = 0; i < server->nr_of_clients; i++) {
        // get clients that should receive packet (everyone but the sender)
        if(id != server->clients[i].id) {
            // specify destination address
            pack_send->address = server->clients[i].addr;
            // send packet to clients
            send_updated_snake_pos(server, pack_recv, pack_send);
        }
    }
}

void handle_color_change(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send)
{
    // format: type id packet_nr new_color
    int type, id, packet_nr, new_color;

    // get client id
    sscanf(pack_recv->data, "%d %d %d", &type, &id, &new_color);
    printf("color change from %d to color %d\n", id, new_color);
    for(int i = 0; i < server->nr_of_clients; i++) {
        // get clients that should receive packet (everyone but the sender)
        if(id != server->clients[i].id) {
            // specify destination address
            pack_send->address = server->clients[i].addr;
            // send packet to clients
            send_color_change(server, pack_recv, pack_send);
        }
    }
}

void handle_collision(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send)
{
    // received data
    int type, id;
    // format: type id
    sscanf(pack_recv->data, "%d %d", &type, &id);

    // update client state on server
    server->clients[id].alive = false;

    // format data to send
    // format: type id
    pack_send->data = pack_recv->data;

    pack_send->channel = pack_recv->channel;
    pack_send->len = sizeof(pack_send->data);
    pack_send->maxlen = 1024;

    // send information to all other clients
    for(int i = 0; i < server->nr_of_clients; i++) {
        pack_send->address = server->clients[i].addr;
        SDLNet_UDP_Send(server->udp_sock, pack_send->channel, pack_send);
    }
}

void handle_ate_fruit(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send)
{
    // received data
    int type, id, fruit_index;
    // format: type client_id fruit_index
    sscanf(pack_recv->data, "%d %d %d", &type, &id, &fruit_index);

    // decrement nr_of_fruits
    server->game_state.nr_of_fruits--;
    // indicate that the fruit index is free
    server->game_state.fruits[fruit_index] = 0;

    // format data to send
    // format: type id (id of player that ate the fruit) fruit_index
    pack_send->data = pack_recv->data;

    pack_send->channel = pack_recv->channel;
    pack_send->len = sizeof(pack_send->data)+8;
    pack_send->maxlen = 1024;

    // send information to all other clients
    for(int i = 0; i < server->nr_of_clients; i++) {
        if(id != server->clients[i].id) {
            printf("----------------\n");
            printf("ate_fruit: type id fruit_index\n");
            log_packet(pack_send);
            printf("----------------\n");

            pack_send->address = server->clients[i].addr;
            SDLNet_UDP_Send(server->udp_sock, pack_send->channel, pack_send);
        }
    }
}

void send_connection_success(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send, unsigned ticks)
{
    // format data
    char msg[128];
    // send back message that the connection was successful, NOTE: increments nr_of_clients
    // format: type id nr_of_clients
    server->nr_of_clients++;
    sprintf(msg, "%d %d %d %u", SUCCESSFUL_CONNECTION, server->clients[server->nr_of_clients-1].id, server->nr_of_clients, ticks);
    pack_send->data = msg;

    pack_send->channel = pack_recv->channel;
    pack_send->len = sizeof(msg);
    pack_send->maxlen = 1024;

    // specify destination address from source address
    pack_send->address = pack_recv->address;

    printf("send_connection_success: type id nr_of_clients\n");
    log_packet(pack_send);

    // send udp packet to client that connected
    SDLNet_UDP_Send(server->udp_sock, pack_send->channel, pack_send);

    // send data to all other connected clients that a new client joined
    // format: type id
    sprintf(msg, "%d %d", NEW_CLIENT_JOINED, server->clients[server->nr_of_clients-1].id);
    pack_send->data = msg;
    for(int i = 0; i < server->nr_of_clients; i++) {
        // get clients that should receive packet (everyone but the sender)
        if(server->clients[server->nr_of_clients-1].id != server->clients[i].id) {
            printf("new_client_joined:\n");
            log_packet(pack_send);
            // specify destination address
            pack_send->address = server->clients[i].addr;
            // send packet to clients
            SDLNet_UDP_Send(server->udp_sock, pack_send->channel, pack_send);
        }
    }
}

void send_connection_failed(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send)
{
    // format data
    char msg[2];
    // send back message that the connection failed
    sprintf(msg, "%d", FAILED_CONNECTION);
    pack_send->data = msg;

    pack_send->channel = pack_recv->channel;
    pack_send->len = sizeof(pack_send->data);
    pack_send->maxlen = 1024;

    // specify destination address from source address
    pack_send->address = pack_recv->address;

    // send udp packet
    SDLNet_UDP_Send(server->udp_sock, pack_send->channel, pack_send);
}

void send_start_game(Server *server, UDPpacket *pack_send)
{
    char msg[128];
    // format: type
    sprintf(msg, "%d", START_GAME);
    pack_send->data = msg;

    pack_send->channel = -1;
    pack_send->len = sizeof(msg);
    pack_send->maxlen = 1024;

    // send to every client
    for(int i = 0; i < server->nr_of_clients; i++) {
        pack_send->address = server->clients[i].addr;
        SDLNet_UDP_Send(server->udp_sock, pack_send->channel, pack_send);
    }
}

void send_updated_snake_pos(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send)
{
    // send the same data that the server received to the other clients
    // send back message that the connection failed
    // format: type id last_received_packet_nr x y direction angle
    int a, id, packet_nr;
    sscanf(pack_recv->data, "%d %d %d", &a, &id, &packet_nr);
    /* printf("sending packet_nr: %d to: %d\n", packet_nr, id); */
    pack_send->data = pack_recv->data;
    pack_send->channel = pack_recv->channel;
    pack_send->len = sizeof(pack_send->data)+24;
    pack_send->maxlen = 1024;

    // destination address is already defined in pack_send
    // send upd packet
    SDLNet_UDP_Send(server->udp_sock, pack_send->channel, pack_send);
    /* send_ticks(server, pack_send); */
}

void send_color_change(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send)
{
    // send the same data that the server received to the other clients
    // send back message that the connection failed
    int a, id, packet_nr;
    sscanf(pack_recv->data, "%d %d %d", &a, &id, &packet_nr);
    /* printf("sending packet_nr: %d to: %d\n", packet_nr, id); */
    pack_send->data = pack_recv->data;
    pack_send->channel = pack_recv->channel;
    pack_send->len = sizeof(pack_send->data)+24;
    pack_send->maxlen = 1024;

    // destination address is already defined in pack_send
    // send upd packet
    log_packet(pack_send);
    SDLNet_UDP_Send(server->udp_sock, pack_send->channel, pack_send);
    /* send_ticks(server, pack_send); */
}

void send_random_fruit_pos(Server *server, UDPpacket *pack_send)
{
    char msg[128];
    int random_x = rand();
    int random_y = rand();
    int random_type = rand();
    int fruit_index = -1;

    // get first free fruit index
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(server->game_state.fruits[i] == 0) {
            // save fruit index
            fruit_index = i;
            // indicate that fruit index is taken
            server->game_state.fruits[i] = 1;
            break;
        }
    }
    printf("fruit_index: %d\n", fruit_index);

    // format request typ, x pos, y pos and random type before sending
    // format: type x_pos y_pos fruit_type fruit_index
    sprintf(msg, "%d %d %d %d %d", RANDOM_POS, random_x, random_y, random_type, fruit_index);

    pack_send->data = msg;
    pack_send->channel = -1;
    pack_send->len = sizeof(msg);
    pack_send->maxlen = 1024;
    
    // send upd packet to each client
    for(int i = 0; i < server->nr_of_clients; i++) {
        pack_send->address = server->clients[i].addr;
        log_packet(pack_send);
        SDLNet_UDP_Send(server->udp_sock, pack_send->channel, pack_send);
    }
}

void send_ticks(Server *server, UDPpacket *pack_send)
{
    // generate new tick
    int ticks = SDL_GetTicks();
    // format message
    char msg[13];
    // format: type ticks
    sprintf(msg, "%d %d", SEND_TICKS, ticks);

    pack_send->data = msg;
    pack_send->channel = -1;
    pack_send->len = sizeof(pack_send->data)+4;
    pack_send->maxlen = 1024;

    /* SDLNet_UDP_Send(server->udp_sock, pack_send->channel, pack_send); */
    // send to all clients
    for(int i = 0; i < server->nr_of_clients; i++) {
        if(!server->clients[i].alive) {
            continue;
        }
        pack_send->address = server->clients[i].addr;
        SDLNet_UDP_Send(server->udp_sock, pack_send->channel, pack_send);
    }
}
