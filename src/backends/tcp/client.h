#pragma once

#include <glib.h>    // for G_DEFINE_AUTOPTR_CLEANUP_FUNC
#include "socket.h"  // for Laik_Tcp_Socket

typedef void Laik_Tcp_ClientFunction (void* data, void* userdata);

typedef struct Laik_Tcp_Client Laik_Tcp_Client;

__attribute__ ((warn_unused_result))
Laik_Tcp_Socket* laik_tcp_client_connect (Laik_Tcp_Client* this, size_t address);

void laik_tcp_client_free (Laik_Tcp_Client* this);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (Laik_Tcp_Client, laik_tcp_client_free)

__attribute__ ((warn_unused_result))
Laik_Tcp_Client* laik_tcp_client_new (Laik_Tcp_ClientFunction* function, void* userdata);

void laik_tcp_client_push (Laik_Tcp_Client* this, void* data);

void laik_tcp_client_store (Laik_Tcp_Client* this, size_t address, Laik_Tcp_Socket* socket);
