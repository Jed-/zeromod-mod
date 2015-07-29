#include <enet/enet.h>
#include "tools.h"
void initialize_enet() {
	enet_initialize();
}
ENetSocket sock = ENET_SOCKET_NULL;
ENetSocket getsock() {
	if(sock != ENET_SOCKET_NULL) return sock;
	sock = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
	enet_socket_set_option(sock, ENET_SOCKOPT_NONBLOCK, 1);
	enet_socket_set_option(sock, ENET_SOCKOPT_BROADCAST, 1);
	return sock;
}
void sendbuf(char *hostname, int port, char *channel, char *_buf) {
	ENetAddress address;
	enet_address_set_host(&address, hostname);
	address.port = port;
	ENetSocket _sock = getsock();
	if(_sock==ENET_SOCKET_NULL) return;
	ENetBuffer buf;
	uchar send[4096];
	ucharbuf p(send, 4096);
	putint(p, 1);
	putint(p, 100);
	sendstring(channel, p);
	sendstring(_buf, p);
	buf.data = send;
	buf.dataLength = p.length();
	enet_socket_send(_sock, &address, &buf, 1);
}
