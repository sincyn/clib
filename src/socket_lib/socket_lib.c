#include "clib/socket_lib.h"
#include <string.h>
#include "clib/memory_lib.h"
#include "socket_internal.h"

bool cl_socket_init(void) { return cl_socket_init_platform(); }

void cl_socket_cleanup(void) { cl_socket_cleanup_platform(); }

cl_socket_t *cl_socket_create(const cl_address_family_t family, const cl_socket_type_t type)
{
    const int handle = cl_socket_create_platform(family, type);
    if (handle == -1)
    {
        return null;
    }

    cl_socket_t *socket = cl_mem_alloc(null, sizeof(cl_socket_t));
    if (socket == null)
    {
        cl_socket_close_platform(handle);
        return null;
    }

    socket->handle = handle;
    socket->family = family;
    socket->type = type;

    return socket;
}

void cl_socket_destroy(cl_socket_t *socket)
{
    if (socket == null)
    {
        return;
    }
    cl_socket_close_platform(socket->handle);
    cl_mem_free(null, socket);
}

bool cl_socket_bind(const cl_socket_t *socket, const cl_socket_address_t *address)
{
    if (socket == null || address == null)
    {
        return false;
    }

    struct sockaddr_storage native_addr;
    socklen_t addr_len;
    cl_socket_to_native_address(address, &native_addr, &addr_len);

    return cl_socket_bind_platform(socket->handle, (struct sockaddr *)&native_addr, addr_len) == 0;
}

bool cl_socket_connect(const cl_socket_t *socket, const cl_socket_address_t *address)
{
    if (socket == null || address == null)
    {
        return false;
    }

    struct sockaddr_storage native_addr;
    socklen_t addr_len;
    cl_socket_to_native_address(address, &native_addr, &addr_len);

    return cl_socket_connect_platform(socket->handle, (struct sockaddr *)&native_addr, addr_len) == 0;
}

bool cl_socket_listen(const cl_socket_t *socket, const int backlog)
{
    if (socket == null)
    {
        return false;
    }

    return cl_socket_listen_platform(socket->handle, backlog) == 0;
}

cl_socket_t *cl_socket_accept(const cl_socket_t *socket, cl_socket_address_t *client_address)
{
    if (socket == null)
    {
        return null;
    }

    struct sockaddr_storage native_addr;
    socklen_t addr_len = sizeof(native_addr);

    const int client_handle = cl_socket_accept_platform(socket->handle, (struct sockaddr *)&native_addr, &addr_len);
    if (client_handle == -1)
    {
        return null;
    }

    cl_socket_t *client_socket = cl_mem_alloc(null, sizeof(cl_socket_t));
    if (client_socket == null)
    {
        cl_socket_close_platform(client_handle);
        return null;
    }

    client_socket->handle = client_handle;
    client_socket->family = socket->family;
    client_socket->type = socket->type;

    if (client_address != null)
    {
        cl_native_to_socket_address((struct sockaddr *)&native_addr, client_address);
    }

    return client_socket;
}

int cl_socket_send(const cl_socket_t *socket, const void *buffer, const u64 length)
{
    if (socket == null || buffer == null)
    {
        return -1;
    }

    return cl_socket_send_platform(socket->handle, buffer, length, 0);
}

int cl_socket_recv(const cl_socket_t *socket, void *buffer, const u64 length)
{
    if (socket == null || buffer == null)
    {
        return -1;
    }

    return cl_socket_recv_platform(socket->handle, buffer, length, 0);
}

int cl_socket_sendto(const cl_socket_t *socket, const void *buffer, const u64 length,
                     const cl_socket_address_t *address)
{
    if (socket == null || buffer == null || address == null)
    {
        return -1;
    }

    struct sockaddr_storage native_addr;
    socklen_t addr_len;
    cl_socket_to_native_address(address, &native_addr, &addr_len);

    return cl_socket_sendto_platform(socket->handle, buffer, length, 0, (struct sockaddr *)&native_addr, addr_len);
}

int cl_socket_recvfrom(const cl_socket_t *socket, void *buffer, const u64 length, cl_socket_address_t *address)
{
    if (socket == null || buffer == null)
    {
        return -1;
    }

    struct sockaddr_storage native_addr;
    socklen_t addr_len = sizeof(native_addr);

    const int received =
        cl_socket_recvfrom_platform(socket->handle, buffer, length, 0, (struct sockaddr *)&native_addr, &addr_len);

    if (received >= 0 && address != null)
    {
        cl_native_to_socket_address((struct sockaddr *)&native_addr, address);
    }

    return received;
}

bool cl_socket_set_option(const cl_socket_t *socket, const int level, const int option_name, const void *option_value,
                          const u64 option_len)
{
    if (socket == null || option_value == null)
    {
        return false;
    }

    return cl_socket_setsockopt_platform(socket->handle, level, option_name, option_value, option_len) == 0;
}

bool cl_socket_get_option(const cl_socket_t *socket, const int level, const int option_name, void *option_value,
                          u64 *option_len)
{
    if (socket == null || option_value == null || option_len == null)
    {
        return false;
    }

    socklen_t native_option_len = *option_len;
    const int result =
        cl_socket_getsockopt_platform(socket->handle, level, option_name, option_value, &native_option_len);
    *option_len = (u64)native_option_len;
    return result == 0;
}

bool cl_socket_set_blocking(const cl_socket_t *socket, bool blocking)
{
    if (socket == null)
    {
        return false;
    }

    return cl_socket_set_blocking_platform(socket->handle, blocking) == 0;
}

bool cl_socket_addr_from_string(const char *ip_address, const uint16_t port, cl_socket_address_t *out_addr)
{
    if (ip_address == null || out_addr == null)
    {
        return false;
    }

    struct in_addr ipv4_addr;
    struct in6_addr ipv6_addr;

    if (inet_pton(AF_INET, ip_address, &ipv4_addr) == 1)
    {
        out_addr->family = CL_AF_INET;
        out_addr->addr.ipv4 = ntohl(ipv4_addr.s_addr);
        out_addr->port = port;
        return true;
    }
    else if (inet_pton(AF_INET6, ip_address, &ipv6_addr) == 1)
    {
        out_addr->family = CL_AF_INET6;
        memcpy(out_addr->addr.ipv6, &ipv6_addr, 16);
        out_addr->port = port;
        return true;
    }

    return false;
}

bool cl_socket_addr_to_string(const cl_socket_address_t *addr, char *ip_string, const u64 ip_string_size,
                              uint16_t *out_port)
{
    if (addr == null || ip_string == null || ip_string_size == 0 || out_port == null)
    {
        return false;
    }

    const char *result = null;

    if (addr->family == CL_AF_INET)
    {
        struct in_addr ipv4_addr;
        ipv4_addr.s_addr = htonl(addr->addr.ipv4);
        result = inet_ntop(AF_INET, &ipv4_addr, ip_string, ip_string_size);
    }
    else if (addr->family == CL_AF_INET6)
    {
        result = inet_ntop(AF_INET6, addr->addr.ipv6, ip_string, ip_string_size);
    }

    if (result == null)
    {
        return false;
    }

    *out_port = addr->port;
    return true;
}

// Utility functions
int cl_family_to_native(const cl_address_family_t family)
{
    switch (family)
    {
    case CL_AF_INET:
        return AF_INET;
    case CL_AF_INET6:
        return AF_INET6;
    default:
        return -1;
    }
}

int cl_type_to_native(const cl_socket_type_t type)
{
    switch (type)
    {
    case CL_SOCK_STREAM:
        return SOCK_STREAM;
    case CL_SOCK_DGRAM:
        return SOCK_DGRAM;
    default:
        return -1;
    }
}

void cl_native_to_socket_address(const struct sockaddr *native_addr, cl_socket_address_t *addr)
{
    if (native_addr->sa_family == AF_INET)
    {
        const struct sockaddr_in *addr_in = (const struct sockaddr_in *)native_addr;
        addr->family = CL_AF_INET;
        addr->addr.ipv4 = ntohl(addr_in->sin_addr.s_addr);
        addr->port = ntohs(addr_in->sin_port);
    }
    else if (native_addr->sa_family == AF_INET6)
    {
        const struct sockaddr_in6 *addr_in6 = (const struct sockaddr_in6 *)native_addr;
        addr->family = CL_AF_INET6;
        memcpy(addr->addr.ipv6, &addr_in6->sin6_addr, 16);
        addr->port = ntohs(addr_in6->sin6_port);
    }
}

void cl_socket_to_native_address(const cl_socket_address_t *addr, struct sockaddr_storage *native_addr,
                                 socklen_t *addr_len)
{
    memset(native_addr, 0, sizeof(*native_addr));

    if (addr->family == CL_AF_INET)
    {
        struct sockaddr_in *addr_in = (struct sockaddr_in *)native_addr;
        addr_in->sin_family = AF_INET;
        addr_in->sin_port = htons(addr->port);
        addr_in->sin_addr.s_addr = htonl(addr->addr.ipv4);
        *addr_len = sizeof(struct sockaddr_in);
    }
    else if (addr->family == CL_AF_INET6)
    {
        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)native_addr;
        addr_in6->sin6_family = AF_INET6;
        addr_in6->sin6_port = htons(addr->port);
        memcpy(&addr_in6->sin6_addr, addr->addr.ipv6, 16);
        *addr_len = sizeof(struct sockaddr_in6);
    }
}
