#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include "server.h"

UDPsocket open_socket(int port)
{
    UDPsocket udp_sock = SDLNet_UDP_Open(port);

    if(!udp_sock) {
        printf("SDLNet_UDP_Open: %s", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    return udp_sock;
}
