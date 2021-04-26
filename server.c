#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <server.h>


int port = 1234;

UDPSocket *udpsocket = open_socket(port);

if (!udpsocket) {
    printf("SDLNet_UDP_Open: %s", SDLNet_GetError());
    return 2;
}
