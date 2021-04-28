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

    UDPpacket *pack_send;
    pack_send = SDLNet_AllocPacket(1024);
    if(!pack_send) {
        fprintf(stderr, "Error: SDLNet_AllocPacket %s\n", SDLNet_GetError());
        return 2;
    }

    // main loop
    while(1) {
        if(SDLNet_UDP_Recv(udp_sock, pack_recv)) {
            // received packet
            handle_received_packet(pack_recv);
            
            //fill packet
            pack_send->channel = pack_recv->channel;
            pack_send->data = pack_recv->data;
            pack_send->len = pack_recv->len+1;
            pack_send->maxlen = 1024;
            pack_send->address = pack_recv->address;

            //send back packet
            SDLNet_UDP_Send(udp_sock, pack_send->channel, pack_send);
        }
    }

    SDLNet_FreePacket(pack_recv);
    SDLNet_FreePacket(pack_send);
    SDLNet_UDP_Close(udp_sock);
    udp_sock = NULL;
    SDLNet_Quit();
    SDL_Quit();

    return 0;
}
