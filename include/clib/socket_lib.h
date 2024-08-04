/**
 * Created by jraynor on 8/3/2024.
 */
#pragma once
#include <clib/defines.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Socket handle
typedef struct cl_socket cl_socket_t;

// Address family
typedef enum cl_address_family
{
    CL_AF_INET, // IPv4
    CL_AF_INET6 // IPv6
} cl_address_family_t;

// Socket type
typedef enum cl_socket_type
{
    CL_SOCK_STREAM, // TCP
    CL_SOCK_DGRAM // UDP
} cl_socket_type_t;

// Socket address structure
typedef struct cl_socket_address
{
    cl_address_family_t family;
    union
    {
        uint32_t ipv4;
        uint8_t ipv6[16];
    } addr;
    uint16_t port;
} cl_socket_address_t;

// Initialize the socket library
bool cl_socket_init(void);

// Cleanup the socket library
void cl_socket_cleanup(void);

// Create a socket
cl_socket_t *cl_socket_create(cl_address_family_t family, cl_socket_type_t type);

// Close and destroy a socket
void cl_socket_destroy(cl_socket_t *socket);

// Bind a socket to an address
bool cl_socket_bind(const cl_socket_t *socket, const cl_socket_address_t *address);

// Connect to a remote address (for TCP)
bool cl_socket_connect(const cl_socket_t *socket, const cl_socket_address_t *address);

// Listen for incoming connections (for TCP server)
bool cl_socket_listen(const cl_socket_t *socket, int backlog);

// Accept an incoming connection (for TCP server)
cl_socket_t *cl_socket_accept(const cl_socket_t *socket, cl_socket_address_t *client_address);

// Send data
int cl_socket_send(const cl_socket_t *socket, const void *buffer, u64 length);

// Receive data
int cl_socket_recv(const cl_socket_t *socket, void *buffer, u64 length);

// Send data to a specific address (for UDP)
int cl_socket_sendto(const cl_socket_t *socket, const void *buffer, u64 length, const cl_socket_address_t *address);

// Receive data and get the sender's address (for UDP)
int cl_socket_recvfrom(const cl_socket_t *socket, void *buffer, u64 length, cl_socket_address_t *address);

// Set socket option
bool cl_socket_set_option(const cl_socket_t *socket, int level, int option_name, const void *option_value,
                          u64 option_len);

// Get socket option
bool cl_socket_get_option(const cl_socket_t *socket, int level, int option_name, void *option_value, u64 *option_len);

// Set socket to blocking or non-blocking mode
bool cl_socket_set_blocking(const cl_socket_t *socket, bool blocking);

// Convert string IP address to cl_socket_address_t
bool cl_socket_addr_from_string(const char *ip_address, uint16_t port, cl_socket_address_t *out_addr);

// Convert cl_socket_address_t to string IP address
bool cl_socket_addr_to_string(const cl_socket_address_t *addr, char *ip_string, u64 ip_string_size, uint16_t *out_port);

#ifdef __cplusplus
}
#endif
