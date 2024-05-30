/** 
 @file  metro.c
 @brief ENet Metro system specific functions
*/
#include "BasePch.hpp"
#if defined( platform_metro )

#include <time.h>
#define ENET_BUILDING_LIB 1
#include "enet/enet.h"

static enet_uint32 timeBase = 0;

int
enet_initialize (void)
{
    return -1;
}

void
enet_deinitialize (void)
{
}

enet_uint32
enet_time_get (void)
{
    return (enet_uint32) GetTickCount64 () - timeBase;
}

void
enet_time_set (enet_uint32 newTimeBase)
{
    timeBase = (enet_uint32) GetTickCount64 () - newTimeBase;
}

int
enet_address_set_host (ENetAddress * address, const char * name)
{
	return -1;
}

int
enet_address_get_host_ip (const ENetAddress * address, char * name, size_t nameLength)
{
	return -1;
}

int
enet_address_get_host (const ENetAddress * address, char * name, size_t nameLength)
{
    return -1;
}

int
enet_socket_bind (ENetSocket socket, const ENetAddress * address)
{
	return -1;
}

int
enet_socket_listen (ENetSocket socket, int backlog)
{
	return -1;
}

ENetSocket
enet_socket_create (ENetSocketType type)
{
    return ENetSocket();
}

ENetSocket 
enet_socket_create_vdp (void)
{
	return ENetSocket();
}

int
enet_socket_set_option (ENetSocket socket, ENetSocketOption option, int value)
{
	return -1;
}

int
enet_socket_connect (ENetSocket socket, const ENetAddress * address)
{
    return -1;
}

ENetSocket
enet_socket_accept (ENetSocket socket, ENetAddress * address)
{
	return ENetSocket();
}

void
enet_socket_destroy (ENetSocket socket)
{
}

int
enet_socket_send (ENetSocket socket,
                  const ENetAddress * address,
                  const ENetBuffer * buffers,
                  size_t bufferCount)
{
	return -1;
}

int
enet_socket_receive (ENetSocket socket,
                     ENetAddress * address,
                     ENetBuffer * buffers,
                     size_t bufferCount)
{
	return -1;
}

int
enet_socketset_select (ENetSocket maxSocket, ENetSocketSet * readSet, ENetSocketSet * writeSet, enet_uint32 timeout)
{
	return -1;
}

int
enet_socket_wait (ENetSocket socket, enet_uint32 * condition, enet_uint32 timeout)
{
	return -1;
} 

#else
void fDontWhineAboutMetroHavingNoSymbolsCompilerBroseph( ){ }

#endif

