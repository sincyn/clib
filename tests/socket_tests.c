#include <stdio.h>
#include <string.h>
#include "clib/log_lib.h"
#include "clib/socket_lib.h"
#include "clib/test_lib.h"
#include "clib/thread_lib.h"
#include "clib/time_lib.h"

#define TEST_PORT 21420 // Changed port number
#define TEST_MESSAGE "Hello, Socket!"
#define BUFFER_SIZE 1024
#define TIMEOUT_MS 5000 // 5 seconds timeout

static cl_thread_t *server_thread;
static cl_thread_t *client_thread;
static bool server_ready = false;
static cl_mutex_t *mutex;
static cl_cond_t *cond;

static char server_received[BUFFER_SIZE];
static char client_received[BUFFER_SIZE];
static bool server_done = false;
static bool client_done = false;

void *server_func(void *arg)
{
    (void)arg;

    cl_socket_t *server_socket = cl_socket_create(CL_AF_INET, CL_SOCK_STREAM);
    if (server_socket == null)
    {
        cl_log_error("Server: Failed to create socket");
        return null;
    }

    cl_socket_address_t addr = {0};
    addr.family = CL_AF_INET;
    addr.addr.ipv4 = 0; // INADDR_ANY
    addr.port = TEST_PORT;

    if (!cl_socket_bind(server_socket, &addr))
    {
        cl_log_error("Server: Failed to bind");
        cl_socket_destroy(server_socket);
        return null;
    }

    if (!cl_socket_listen(server_socket, 5))
    {
        cl_log_error("Server: Failed to listen");
        cl_socket_destroy(server_socket);
        return null;
    }


    cl_mutex_lock(mutex);
    server_ready = true;
    cl_cond_broadcast(cond); // Changed to broadcast
    cl_mutex_unlock(mutex);

    cl_log_debug("Server: Waiting for client connection");
    cl_socket_t *client_socket = cl_socket_accept(server_socket, null);
    if (client_socket == null)
    {
        cl_log_error("Server: Failed to accept client connection");
        cl_socket_destroy(server_socket);
        return null;
    }

    cl_log_debug("Server: Client connected, receiving data");
    int received = cl_socket_recv(client_socket, server_received, BUFFER_SIZE - 1);
    if (received <= 0)
    {
        cl_log_warn("Server: Failed to receive data");
    }
    else
    {
        server_received[received] = '\0';
        cl_log_debug("Server: Received '%s'", server_received);

        if (cl_socket_send(client_socket, server_received, strlen(server_received)) <= 0)
        {
            cl_log_warn("Server: Failed to send data");
        }
    }

    cl_socket_destroy(client_socket);
    cl_socket_destroy(server_socket);

    cl_mutex_lock(mutex);
    server_done = true;
    cl_cond_signal(cond);
    cl_mutex_unlock(mutex);

    return null;
}

void *client_func(void *arg)
{
    (void)arg;

    cl_mutex_lock(mutex);
    cl_time_t start_time, current_time;
    cl_time_get_current(&start_time);

    while (!server_ready)
    {
        cl_log_info("Client: Waiting for server to be ready");
        if (!cl_cond_timedwait(cond, mutex, 1000)) // Wait for 1 second at a time
        {
            cl_time_get_current(&current_time);
            if (cl_time_to_ms(&current_time) - cl_time_to_ms(&start_time) >= TIMEOUT_MS)
            {
                cl_log_info("Client: Timeout waiting for server");
                cl_mutex_unlock(mutex);
                return null;
            }
        }
    }
    cl_mutex_unlock(mutex);

    cl_log_debug("Client: Server is ready, creating socket");
    cl_socket_t *client_socket = cl_socket_create(CL_AF_INET, CL_SOCK_STREAM);
    if (client_socket == null)
    {
        cl_log_info("Client: Failed to create socket");
        return null;
    }

    cl_socket_address_t addr = {0};
    if (!cl_socket_addr_from_string("127.0.0.1", TEST_PORT, &addr))
    {
        cl_log_info("Client: Failed to create address");
        cl_socket_destroy(client_socket);
        return null;
    }

    if (!cl_socket_connect(client_socket, &addr))
    {
        cl_log_info("Client: Failed to connect");
        cl_socket_destroy(client_socket);
        return null;
    }

    cl_log_info("Client: Connected, sending message");
    if (cl_socket_send(client_socket, TEST_MESSAGE, strlen(TEST_MESSAGE)) <= 0)
    {
        cl_log_info("Client: Failed to send data");
    }
    else
    {
        cl_log_info("Client: Message sent, waiting for response");
        int received = cl_socket_recv(client_socket, client_received, BUFFER_SIZE - 1);
        if (received <= 0)
        {
            cl_log_info("Client: Failed to receive data");
        }
        else
        {
            client_received[received] = '\0';
            cl_log_info("Client: Received '%s'", client_received);
        }
    }

    cl_socket_destroy(client_socket);

    cl_mutex_lock(mutex);
    client_done = true;
    cl_cond_signal(cond);
    cl_mutex_unlock(mutex);

    cl_log_info("Client: Done");
    return null;
}

CL_TEST(test_socket_full_communication)
{
    cl_log_info("Starting full communication test");

    mutex = cl_mutex_create();
    cond = cl_cond_create();

    server_ready = false;
    server_done = false;
    client_done = false;
    memset(server_received, 0, BUFFER_SIZE);
    memset(client_received, 0, BUFFER_SIZE);
    server_thread = cl_thread_create(server_func, null, 0);
    client_thread = cl_thread_create(client_func, null, 0);

    cl_time_t start_time, current_time;
    cl_time_get_current(&start_time);

    cl_mutex_lock(mutex);
    while ((!server_done || !client_done) && cl_time_to_ms(&current_time) - cl_time_to_ms(&start_time) < TIMEOUT_MS)
    {
        cl_log_info("Waiting for threads to complete (Server: %s, Client: %s)", server_done ? "Done" : "Not Done",
                    client_done ? "Done" : "Not Done");
        cl_cond_timedwait(cond, mutex, 1000); // Wait for 1 second at a time
        cl_time_get_current(&current_time);
    }
    cl_mutex_unlock(mutex);

    cl_log_info("Threads complete or timeout reached");

    cl_thread_join(server_thread, null);
    cl_thread_join(client_thread, null);
    cl_thread_destroy(server_thread);
    cl_thread_destroy(client_thread);

    cl_mutex_destroy(mutex);
    cl_cond_destroy(cond);

    cl_log_info("Asserting results");
    CL_ASSERT_EQUAL(server_done, true);
    CL_ASSERT_EQUAL(client_done, true);
    CL_ASSERT_STRING_EQUAL(server_received, TEST_MESSAGE);
    CL_ASSERT_STRING_EQUAL(client_received, TEST_MESSAGE);

    cl_log_info("Full communication test complete");
}


CL_TEST(test_socket_create)
{
    cl_socket_t *socket = cl_socket_create(CL_AF_INET, CL_SOCK_STREAM);
    CL_ASSERT_NOT_NULL(socket);
    cl_socket_destroy(socket);
}

CL_TEST(test_socket_bind)
{
    cl_socket_t *socket = cl_socket_create(CL_AF_INET, CL_SOCK_STREAM);
    cl_socket_address_t addr = {0};
    addr.family = CL_AF_INET;
    addr.addr.ipv4 = 0; // INADDR_ANY
    addr.port = TEST_PORT + 1; // Use a different port to avoid conflicts

    bool result = cl_socket_bind(socket, &addr);
    CL_ASSERT_EQUAL(result, true);

    cl_socket_destroy(socket);
}

CL_TEST(test_socket_listen)
{
    cl_socket_t *socket = cl_socket_create(CL_AF_INET, CL_SOCK_STREAM);
    cl_socket_address_t addr = {0};
    addr.family = CL_AF_INET;
    addr.addr.ipv4 = 0; // INADDR_ANY
    addr.port = TEST_PORT + 2; // Use a different port to avoid conflicts

    cl_socket_bind(socket, &addr);
    bool result = cl_socket_listen(socket, 5);
    CL_ASSERT_EQUAL(result, true);

    cl_socket_destroy(socket);
}

CL_TEST_SUITE_BEGIN(SocketTests)
CL_TEST_SUITE_TEST(test_socket_full_communication)
CL_TEST_SUITE_TEST(test_socket_create)
CL_TEST_SUITE_TEST(test_socket_bind)
CL_TEST_SUITE_TEST(test_socket_listen)
CL_TEST_SUITE_END

int main()
{
    cl_log_init_default(CL_LOG_ERROR);
    cl_socket_init();
    CL_RUN_TEST_SUITE(SocketTests);
    CL_RUN_ALL_TESTS();
    cl_socket_cleanup();
    cl_log_cleanup();
    return 0;
}
