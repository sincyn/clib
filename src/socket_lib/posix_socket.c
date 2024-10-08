/**
 * Created by jraynor on 8/3/2024.
 */
#ifndef _WIN32

#include "socket_internal.h"

bool cl_socket_init_platform(void)
{
    return true; // No initialization needed for POSIX sockets
}

void cl_socket_cleanup_platform(void)
{
    // No cleanup needed for POSIX sockets
}

int cl_socket_create_platform(cl_address_family_t family, cl_socket_type_t type)
{
    return socket(cl_family_to_native(family), cl_type_to_native(type), 0);
}

void cl_socket_close_platform(int handle) { close(handle); }

int cl_socket_bind_platform(int handle, const struct sockaddr *addr, socklen_t addrlen)
{
    return bind(handle, addr, addrlen);
}

int cl_socket_connect_platform(int handle, const struct sockaddr *addr, socklen_t addrlen)
{
    return connect(handle, addr, addrlen);
}

int cl_socket_listen_platform(int handle, int backlog) { return listen(handle, backlog); }

int cl_socket_accept_platform(int handle, struct sockaddr *addr, socklen_t *addrlen)
{
    return accept(handle, addr, addrlen);
}

int cl_socket_send_platform(int handle, const void *buf, u64 len, int flags)
{
    return (int)send(handle, buf, len, flags);
}

int cl_socket_recv_platform(int handle, void *buf, u64 len, int flags) { return (int)recv(handle, buf, len, flags); }

int cl_socket_sendto_platform(int handle, const void *buf, u64 len, int flags, const struct sockaddr *dest_addr,
                              socklen_t addrlen)
{
    return (int)sendto(handle, buf, len, flags, dest_addr, addrlen);
}

int cl_socket_recvfrom_platform(int handle, void *buf, u64 len, int flags, struct sockaddr *src_addr,
                                socklen_t *addrlen)
{
    return (int)recvfrom(handle, buf, len, flags, src_addr, addrlen);
}

int cl_socket_setsockopt_platform(int handle, int level, int optname, const void *optval, socklen_t optlen)
{
    return setsockopt(handle, level, optname, optval, optlen);
}

int cl_socket_getsockopt_platform(int handle, int level, int optname, void *optval, socklen_t *optlen)
{
    return getsockopt(handle, level, optname, optval, optlen);
}

int cl_socket_set_blocking_platform(int handle, bool blocking)
{
    int flags = fcntl(handle, F_GETFL, 0);
    if (flags == -1)
        return -1;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return fcntl(handle, F_SETFL, flags);
}

#endif // !_WIN32
