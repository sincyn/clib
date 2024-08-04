#pragma once

#include "../../include/clib/socket_lib.h"

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>

    // Link with ws2_32.lib
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
#endif

struct cl_socket {
#ifdef _WIN32
    SOCKET handle;
#else
    int handle;
#endif
    cl_address_family_t family;
    cl_socket_type_t type;
};

// Platform-specific function declarations
bool cl_socket_init_platform(void);
void cl_socket_cleanup_platform(void);
int cl_socket_create_platform(cl_address_family_t family, cl_socket_type_t type);
void cl_socket_close_platform(int handle);
int cl_socket_bind_platform(int handle, const struct sockaddr *addr, socklen_t addrlen);
int cl_socket_connect_platform(int handle, const struct sockaddr *addr, socklen_t addrlen);
int cl_socket_listen_platform(int handle, int backlog);
int cl_socket_accept_platform(int handle, struct sockaddr *addr, socklen_t *addrlen);
int cl_socket_send_platform(int handle, const void *buf, u64 len, int flags);
int cl_socket_recv_platform(int handle, void *buf, u64 len, int flags);
int cl_socket_sendto_platform(int handle, const void *buf, u64 len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
int cl_socket_recvfrom_platform(int handle, void *buf, u64 len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
int cl_socket_setsockopt_platform(int handle, int level, int optname, const void *optval, socklen_t optlen);
int cl_socket_getsockopt_platform(int handle, int level, int optname, void *optval, socklen_t *optlen);
int cl_socket_set_blocking_platform(int handle, bool blocking);

// Utility functions
int cl_family_to_native(cl_address_family_t family);
int cl_type_to_native(cl_socket_type_t type);
void cl_native_to_socket_address(const struct sockaddr *native_addr, cl_socket_address_t *addr);
void cl_socket_to_native_address(const cl_socket_address_t *addr, struct sockaddr_storage *native_addr, socklen_t *addr_len);