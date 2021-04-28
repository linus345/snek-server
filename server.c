#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include "server.h"

enum Request_Type {
    JOIN = 0,
    UPDATE_SNAKE_POS = 1
};

UDPsocket open_socket(int port)
{
    UDPsocket udp_sock = SDLNet_UDP_Open(port);

    if(!udp_sock) {
        printf("SDLNet_UDP_Open: %s", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    return udp_sock;
}

void handle_received_packet(UDPpacket *pack_recv)
{
    int type;
    // get request type from packet
    sscanf(pack_recv->data, "%d", &type);

    switch(type) {
        case JOIN:
            printf("join: %s\n", pack_recv->data);
            break;
        case UPDATE_SNAKE_POS:
            printf("update_snake_pos: %s\n", pack_recv->data);
            break;
    }
    /* printf("Channel: %d\n", pack_recv->channel); */
    /* printf("Data: %s\n", pack_recv->data); */
    /* printf("Len: %d\n", pack_recv->len); */
    /* printf("Max len: %d\n", pack_recv->maxlen); */
    /* printf("Status: %d\n", pack_recv->status); */
    /* printf("src host: %u\n", pack_recv->address.host); */
    /* printf("src port: %u\n", pack_recv->address.port); */
}
