/** 
 @file  win32.h
 @brief ENet Win32 header
*/
#ifndef __ENET_METRO_H__
#define __ENET_METRO_H__

#ifdef ENET_BUILDING_LIB
#pragma warning (disable: 4996) // 'strncpy' was declared deprecated
#pragma warning (disable: 4267) // size_t to int conversion
#pragma warning (disable: 4244) // 64bit to 32bit int
#pragma warning (disable: 4146) // unary minus operator applied to unsigned type, result still unsigned
#pragma warning (disable: 4018) // signed/unsigned mismatch
#endif

#include <stdlib.h>

typedef int ENetSocket;

enum
{
    ENET_SOCKET_NULL = 0
};

#define ENET_HOST_TO_NET_16(value) (value)
#define ENET_HOST_TO_NET_32(value) (value)

#define ENET_NET_TO_HOST_16(value) (value)
#define ENET_NET_TO_HOST_32(value) (value)

typedef struct
{
    size_t dataLength;
    void * data;
} ENetBuffer;

#define ENET_CALLBACK __cdecl

#if defined ENET_DLL
#if defined ENET_BUILDING_LIB
#define ENET_API __declspec( dllexport )
#else
#define ENET_API __declspec( dllimport )
#endif /* ENET_BUILDING_LIB */
#else /* !ENET_DLL */
#define ENET_API extern
#endif /* ENET_DLL */

typedef void* ENetSocketSet; // ???

#define ENET_SOCKETSET_EMPTY(sockset)          FD_ZERO (& (sockset))
#define ENET_SOCKETSET_ADD(sockset, socket)    FD_SET (socket, & (sockset))
#define ENET_SOCKETSET_REMOVE(sockset, socket) FD_CLEAR (socket, & (sockset))
#define ENET_SOCKETSET_CHECK(sockset, socket)  FD_ISSET (socket, & (sockset))

#endif /* __ENET_METRO_H__ */


