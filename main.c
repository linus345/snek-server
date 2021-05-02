#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

    if(argc != 3) {
        printf("Usage: %s <host> <port>\n", argv[0]);
        return 1;
    }
    
    // initialize server structure with correct host and port
    char *host = argv[1];
    int port = atoi(argv[2]);
    Server *server = init_server(host, port);

    // open udp socket
    server->udp_sock = open_socket(server->port);

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

    srand(time(NULL));
    unsigned last_time = 0, current_time;

    // main loop
    while(1) {
        if(SDLNet_UDP_Recv(server->udp_sock, pack_recv)) {
            // received packet, send occurs inside this function too
            handle_received_packet(server, pack_recv, pack_send);
        }

        // generate new fruit position every other second and send to clients
        current_time = SDL_GetTicks();
        if (current_time > last_time + 2000) {
            // send random fruit position to all clients
            send_random_fruit_pos(server, pack_send);
            last_time = current_time;
        }  
    }

    SDLNet_FreePacket(pack_recv);
    SDLNet_FreePacket(pack_send);
    SDLNet_UDP_Close(server->udp_sock);
    server->udp_sock = NULL;
    free(server);
    SDLNet_Quit();
    SDL_Quit();

    return 0;
}
