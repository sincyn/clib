/**
 * Created by jraynor on 8/3/2024.
 */
#ifdef _WIN32

#include "socket_internal.h"

bool cl_socket_init_platform(void)
{
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0;
}

void cl_socket_cleanup_platform(void) { WSACleanup(); }

int cl_socket_create_platform(cl_address_family_t family, cl_socket_type_t type)
{
    return (int)WSASocket(cl_family_to_native(family), cl_type_to_native(type), 0, NULL, 0, WSA_FLAG_OVERLAPPED);
}

void cl_socket_close_platform(int handle) { closesocket((SOCKET)handle); }

int cl_socket_bind_platform(int handle, const struct sockaddr *addr, socklen_t addrlen)
{
    return bind((SOCKET)handle, addr, addrlen);
}

int cl_socket_connect_platform(int handle, const struct sockaddr *addr, socklen_t addrlen)
{
    return connect((SOCKET)handle, addr, addrlen);
}

int cl_socket_listen_platform(int handle, int backlog) { return listen((SOCKET)handle, backlog); }

int cl_socket_accept_platform(int handle, struct sockaddr *addr, socklen_t *addrlen)
{
    return (int)accept((SOCKET)handle, addr, addrlen);
}

int cl_socket_send_platform(int handle, const void *buf, size_t len, int flags)
{
    return send((SOCKET)handle, buf, (int)len, flags);
}

int cl_socket_recv_platform(int handle, void *buf, size_t len, int flags)
{
    return recv((SOCKET)handle, buf, (int)len, flags);
}

int cl_socket_sendto_platform(int handle, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr,
                              socklen_t addrlen)
{
    return sendto((SOCKET)handle, buf, (int)len, flags, dest_addr, addrlen);
}

int cl_socket_recvfrom_platform(int handle, void *buf, size_t len, int flags, struct sockaddr *src_addr,
                                socklen_t *addrlen)
{
    return recvfrom((SOCKET)handle, buf, (int)len, flags, src_addr, addrlen);
}

int cl_socket_setsockopt_platform(int handle, int level, int optname, const void *optval, socklen_t optlen)
{
    return setsockopt((SOCKET)handle, level, optname, optval, optlen);
}

int cl_socket_getsockopt_platform(int handle, int level, int optname, void *optval, socklen_t *optlen)
{
    return getsockopt((SOCKET)handle, level, optname, optval, optlen);
}

int cl_socket_set_blocking_platform(int handle, bool blocking)
{
    u_long mode = blocking ? 0 : 1;
    return ioctlsocket((SOCKET)handle, FIONBIO, &mode);
}

#endif // _WIN32
