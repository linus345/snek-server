#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include "server.h"

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error: SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    printf("successfully initialized SDL\n");

    if (SDLNet_Init() != 0) {
        fprintf(stderr, "Error: SDLNet_Init %s\n", SDLNet_GetError());
        return 2;
    }
    printf("succesfully initialized SDL_net\n");

    int port = 1234;
    // open udp socket
    UDPsocket udp_sock = open_socket(port);

    // allocate memory for receive packet
    UDPpacket *pack_recv;
    pack_recv = SDLNet_AllocPacket(1024);
    if(!pack_recv) {
        fprintf(stderr, "Error: SDLNet_AllocPacket %s\n", SDLNet_GetError());
        return 2;
    }

    // main loop
    while(1) {
        if(SDLNet_UDP_Recv(udp_sock, pack_recv)) {
            // received packet
            printf("Channel: %d\n", pack_recv->channel);
            printf("Data: %s\n", pack_recv->data);
            printf("Len: %d\n", pack_recv->len);
            printf("Max len: %d\n", pack_recv->maxlen);
            printf("Status: %d\n", pack_recv->status);
            printf("src host: %u\n", pack_recv->address.host);
            printf("src port: %u\n", pack_recv->address.port);
        }
    }

    SDLNet_FreePacket(pack_recv);
    SDLNet_UDP_Close(udp_sock);
    SDLNet_Quit();
    SDL_Quit();

    return 0;
}
