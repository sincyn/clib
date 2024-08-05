/**
 * Created by jraynor on 8/3/2024.
 */
/**
 * Created by Claude on 8/3/2024.
 */
#include <stdio.h>
#include <string.h>
#include "clib/log_lib.h"
#include "clib/socket_lib.h"

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define CL_SOCKET_ADDR_STRING_SIZE 46

void run_server()
{
    cl_socket_t *server_socket = cl_socket_create(CL_AF_INET, CL_SOCK_STREAM);
    if (server_socket == NULL)
    {
        cl_log_error("Failed to create server socket");
        return;
    }

    cl_socket_address_t server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.family = CL_AF_INET;
    server_addr.addr.ipv4 = 0; // INADDR_ANY
    server_addr.port = SERVER_PORT;

    if (!cl_socket_bind(server_socket, &server_addr))
    {
        cl_log_error("Failed to bind server socket");
        cl_socket_destroy(server_socket);
        return;
    }

    if (!cl_socket_listen(server_socket, 5))
    {
        cl_log_error("Failed to listen on server socket");
        cl_socket_destroy(server_socket);
        return;
    }

    cl_log_info("Server listening on port %d", SERVER_PORT);

    cl_socket_address_t client_addr;
    cl_socket_t *client_socket = cl_socket_accept(server_socket, &client_addr);
    if (client_socket == NULL)
    {
        cl_log_error("Failed to accept client connection");
        cl_socket_destroy(server_socket);
        return;
    }

    char client_ip[CL_SOCKET_ADDR_STRING_SIZE];
    uint16_t client_port;
    if (cl_socket_addr_to_string(&client_addr, client_ip, sizeof(client_ip), &client_port))
    {
        cl_log_info("Client connected from %s:%d", client_ip, client_port);
    }

    char buffer[BUFFER_SIZE];
    int bytes_received = cl_socket_recv(client_socket, buffer, BUFFER_SIZE - 1);
    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        cl_log_info("Received from client: %s", buffer);

        const char *response = "Hello from server!";
        cl_socket_send(client_socket, response, strlen(response));
    }

    cl_socket_destroy(client_socket);
    cl_socket_destroy(server_socket);
}

void run_client()
{
    cl_socket_t *client_socket = cl_socket_create(CL_AF_INET, CL_SOCK_STREAM);
    if (client_socket == NULL)
    {
        cl_log_error("Failed to create client socket");
        return;
    }

    cl_socket_address_t server_addr;
    if (!cl_socket_addr_from_string("127.0.0.1", SERVER_PORT, &server_addr))
    {
        cl_log_error("Failed to create server address");
        cl_socket_destroy(client_socket);
        return;
    }

    if (!cl_socket_connect(client_socket, &server_addr))
    {
        cl_log_error("Failed to connect to server");
        cl_socket_destroy(client_socket);
        return;
    }

    cl_log_info("Connected to server");

    const char *message = "Hello from client!";
    cl_socket_send(client_socket, message, strlen(message));

    char buffer[BUFFER_SIZE];
    int bytes_received = cl_socket_recv(client_socket, buffer, BUFFER_SIZE - 1);
    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        cl_log_info("Received from server: %s", buffer);
    }

    cl_socket_destroy(client_socket);
}

int main(int argc, char *argv[])
{
    cl_log_config_t log_config = {.include_timestamp = true, .include_level = true, .include_file_line = true};
    cl_log_init(&log_config);

    cl_log_target_config_t console_config = {
        .type = CL_LOG_TARGET_CONSOLE, .min_level = CL_LOG_INFO, .config.console = {.use_colors = true}};
    cl_log_add_target(&console_config);

    if (!cl_socket_init())
    {
        cl_log_error("Failed to initialize socket library");
        return 1;
    }

    if (argc > 1 && strcmp(argv[1], "server") == 0)
    {
        run_server();
    }
    else
    {
        run_client();
    }

    cl_socket_cleanup();
    cl_log_cleanup();

    return 0;
}
