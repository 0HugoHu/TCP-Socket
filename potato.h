//
// Created by Hugo-L on 2023/2/18.
//

#ifndef TCP_SOCKET_POTATO_H
#define TCP_SOCKET_POTATO_H

#include <assert.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>

#define PORT 5477


struct potato_struct {
    int num_hops;
    int current_num;
    int traces[520]; // range in [0, 512]
    int is_over; // 1 for over, and 0 otherwise
};
typedef struct potato_struct potato_t;

typedef struct sockaddr_storage sockaddr_t;


int send_s(int socket_fd, const void *ptr, size_t size, int flag) {
    int bytes = 0;
    int recval = 0;
    while (bytes != size) {
        recval = send(socket_fd, ptr, size, flag);
        if (recval == -1) {
            std::cerr << "Send failure!" << std::endl;
            return 1; // send fails
        }
        bytes += recval;
    }
    return 0; // send succeeds
}

// Check input format
int check_cli_argv(int argc, int expected_argc) {
    if (argc != expected_argc) {
        std::cerr << "Invalid arguments provided!" << std::endl;
        return 1;
    }
    return 0;
}

// Check for socket-related function returns
int check_status(int status, int error_status, int case_id, int not_equal = 0) {
    const char *prompt[] = {"Error: cannot get address info for host!",
                            "Error: cannot create socket!",
                            "Error: cannot connect/bind to socket!",
                            "Error: cannot listen on socket!",
                            "Error: cannot accept connection on the socket of player!",
                            "Error: player's connection with the left is down!",
                            "Error: cannot accept connection on the socket from player!",
                            "Error: building connections between player's neighbor failed!",
                            "Error: select()!",
                            "Error: player's side is down!"};
    if ((not_equal && status != error_status) || (!not_equal && status == error_status)) {
        std::cerr << prompt[case_id] << std::endl;
        return 1;
    }
    return 0;
}

// Convert port number to string indicating left or right
void convert_port(int id, int num_players, char port_str[], int left = 0) {
    int port = 0;
    // Move right for 1
    if (left == 0)
        port = 1 + PORT + (num_players + id - 1) % num_players;
    else
        port = 1 + PORT + (num_players + id) % num_players;
    assert(sprintf(port_str, "%d", port) >= 0);
}

// Establish connection or bind listener between player and its left or right
int establish_connection(int *socket_todo, const char *port, int is_connect = true, const char *hostname = NULL,
                         int is_passive = false, int retry = false) {
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    // Allocate memory
    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    if (is_passive)
        host_info.ai_flags = AI_PASSIVE;

    // Check for obtaining address
    if (check_status(getaddrinfo(hostname, port, &host_info, &host_info_list), 0, 0, 1)) return 1;

    // Check for socket creation
    if (check_status(
            *socket_todo = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol),
            -1, 1))
        return 1;

    // Check for socket connection or binding
    if (!retry) {
        if (is_connect &&
            check_status(connect(*socket_todo, host_info_list->ai_addr, host_info_list->ai_addrlen), -1, 2))
            return 1;
        if (!is_connect) {
            int temp = 1;
            setsockopt(*socket_todo, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(temp));
            if (check_status(bind(*socket_todo, host_info_list->ai_addr, host_info_list->ai_addrlen), -1, 2)) return 1;
        }
    } else {
        int status = -1;
        while (status == -1) {
            status = connect(*socket_todo, host_info_list->ai_addr, host_info_list->ai_addrlen);
        }
    }
    freeaddrinfo(host_info_list);
    return 0;
}

#endif //TCP_SOCKET_POTATO_H
