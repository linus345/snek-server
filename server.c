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

void handle_received_packet(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send)
{
    int type;
    // get request type from packet
    sscanf(pack_recv->data, "%d", &type);

    switch(type) {
        case CLIENT_JOIN:
            handle_join_request(server, pack_recv, pack_send);
            break;
        case UPDATE_SNAKE_POS:
            handle_update_snake_pos(server, pack_recv, pack_send);
            break;
    }
}

void handle_join_request(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send)
{
    if(server->nr_of_clients < MAX_CLIENTS) {
        // client successfully joined server
        // give client id
        server->clients[server->nr_of_clients].id = server->nr_of_clients;
        // add client ip to connected clients, increment of nr_of_clients is done in send_connection_success
        server->clients[server->nr_of_clients].addr = pack_recv->address;
        printf("client connected\n");
        log_packet(pack_recv);
        // send back connection success
        send_connection_success(server, pack_recv, pack_send);
    } else {
        // already 4 clients connected so connection failed
        // send back connection failure
        send_connection_failed(server, pack_recv, pack_send);
    }
}

void handle_update_snake_pos(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send)
{
    int id, temp;
    printf("update_snake_pos: %s\n", pack_recv->data);

    for(int i = 0; i < server->nr_of_clients; i++) {
        // get client id
        sscanf(pack_recv->data, "%d %d", &temp, &id);
        printf("temp: %d, id: %d\n", temp, id);
        // get clients that should receive packet (everyone but the sender)
        if(id != server->clients[i].id) {
            // specify destination address
            pack_send->address = server->clients[i].addr;
            // send packet to clients
            send_updated_snake_pos(server, pack_recv, pack_send);
        }
    }
}

void send_connection_success(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send)
{
    // format data
    char msg[4];
    // send back message that the connection was successful, NOTE: increments nr_of_clients
    // format: type id
    sprintf(msg, "%d %d", SUCCESSFUL_CONNECTION, server->clients[server->nr_of_clients++].id);
    pack_send->data = msg;

    pack_send->channel = pack_recv->channel;
    pack_send->len = sizeof(pack_send->data);
    pack_send->maxlen = 1024;

    // specify destination address from source address
    pack_send->address = pack_recv->address;

    log_packet(pack_send);
    // send udp packet
    SDLNet_UDP_Send(server->udp_sock, pack_send->channel, pack_send);
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

void send_updated_snake_pos(Server *server, UDPpacket *pack_recv, UDPpacket *pack_send)
{
    // send the same data that the server received to the other clients
    // send back message that the connection failed
    // format: type x y
    pack_send->data = pack_recv->data;

    pack_send->channel = pack_recv->channel;
    pack_send->len = sizeof(pack_send->data);
    pack_send->maxlen = 1024;

    // destination address is already defined in pack_send
    // send upd packet
    SDLNet_UDP_Send(server->udp_sock, pack_send->channel, pack_send);
}
