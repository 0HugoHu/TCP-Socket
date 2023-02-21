#include "potato.h"

int main(int argc, char *argv[]) {
    // Check for cli arguments
    if (check_cli_argv(argc, 4)) return 1;

    int status;
    int socket_fd;
    const char *hostname = NULL;
    const char *port = argv[1];
    int num_players = std::atoi(argv[2]);
    int num_hops = std::atoi(argv[3]);

    if (num_players <= 1) {
        std::cout << "Error: num_players must be greater than 1." << std::endl;
        return 1;
    }
    if (num_hops < 0 || num_hops > 512) {
        std::cout << "Error: num_hops must in the range [0, 512]." << std::endl;
        return 1;
    }

    std::cout << "Potato Ringmaster" << std::endl;
    std::cout << "Players = " << num_players << std::endl;
    std::cout << "Hops = " << num_hops << std::endl;

    // Listen to port
    if (establish_connection(&socket_fd, port, false, hostname, true)) return 1;
    if (check_status(listen(socket_fd, 100), -1, 3)) return 1;

    // Store each player's info
    int socket_array[num_players];
    sockaddr_t socket_addr_list[num_players];
    socklen_t socket_addr_len_list[num_players];

    // Connect with every single player
    int counter = 0;
    while (counter < num_players) {
        sockaddr_t socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);
        socket_array[counter] = accept(socket_fd, (struct sockaddr *) &socket_addr, &socket_addr_len);

        if (check_status(socket_array[counter], -1, 6)) return 1;
        socket_addr_list[counter] = socket_addr;
        socket_addr_len_list[counter] = socket_addr_len;

        int connectedFlag = 0;
        assert(recv(socket_array[counter], &connectedFlag, sizeof(connectedFlag), MSG_WAITALL) ==
               sizeof(connectedFlag));

        if (connectedFlag == 1) {
            std::cout << "Player " << counter << " is ready to play" << std::endl;
            int numberMessage[2] = {counter, num_players};
            if (send_s(socket_array[counter], &numberMessage, sizeof(numberMessage), 0)) return 1;
        } else {
            if (check_status(0, 0, 9)) return 1;
        }

        counter++;
    }

    // Send neighbors to player
    for (int i = 0; i < num_players; i++) {
        int to_listen = 1;
        int to_connect = 2;
        // Send instruction to listen
        if (send_s(socket_array[i], &to_listen, sizeof(to_listen), 0)) return 1;

        // Send instruction to the left client to connect
        int prev_index = (i - 1 < 0) ? i + num_players - 1 : i - 1;

        if (send_s(socket_array[prev_index], &to_connect, sizeof(to_connect), 0)) return 1;
        if (send_s(socket_array[prev_index], &socket_addr_len_list[i], sizeof(socklen_t), 0)) return 1;
        if (send_s(socket_array[prev_index], &socket_addr_list[i], socket_addr_len_list[i], 0)) return 1;

        int successFlag = 0;
        assert(recv(socket_array[i], &successFlag, sizeof(successFlag), MSG_WAITALL) == sizeof(successFlag));
        if (check_status(successFlag, 1, 7, true)) return 1;
    }


    // Synchronize ready
    for (int i = 0; i < num_players; i++) {
        int synFlag = 0;
        if (send_s(socket_array[i], &synFlag, sizeof(synFlag), 0)) return 1;
    }

    // Initialize potato
    potato_t potato = {.num_hops = num_hops, .current_num = -1, .is_over = 0};

    // Random selection
    srand((unsigned int) time(NULL) + num_players);
    int random_start = rand() % num_players;

    // Send the first potato
    if (send_s(socket_array[random_start], &potato, sizeof(potato), 0)) return 1;
    std::cout << "Ready to start the game, sending potato to player " << random_start << std::endl;

    // Select where the potato coming back
    fd_set rfds;
    int recval;
    int max_fdid = -1;

    FD_ZERO(&rfds);
    // Add all the sockets to the set
    for (int i = 0; i < num_players; i++) {
        FD_SET(socket_array[i], &rfds);
        max_fdid = (socket_array[i] > max_fdid) ? socket_array[i] : max_fdid;
    }

    // Wait for the potato to come back
    recval = select(max_fdid + 1, &rfds, NULL, NULL, NULL);
    if (check_status(recval, -1, 8)) return 1;

    for (int i = 0; i < num_players; i++) {
        if (FD_ISSET(socket_array[i], &rfds)) {
            assert(recv(socket_array[i], &potato, sizeof(potato), MSG_WAITALL) == sizeof(potato));
            break;
        }
    }
    FD_ZERO(&rfds);

    // Receive from it and print the results
    std::cout << "Trace of potato:" << std::endl;
    for (int i = 0; i < num_hops; i++) {
        if (i == 0)
            std::cout << potato.traces[i];
        else
            std::cout << "," << potato.traces[i];
    }
    std::cout << std::endl;

    // Tell all players game is over
    potato.is_over = 1;
    for (int i = 0; i < num_players; i++) {
        if (send_s(socket_array[i], &potato, sizeof(potato), 0)) return 1;
    }

    // Synchronize all players to game over state
    for (int i = 0; i < num_players; i++) {
        int gameOverFlag = 0;
        assert(recv(socket_array[i], &gameOverFlag, sizeof(gameOverFlag), MSG_WAITALL) == sizeof(gameOverFlag));
        assert(gameOverFlag == 1);
    }

    // Tell all players to close sockets
    for (int i = 0; i < num_players; i++) {
        int closedFlag = 1;
        if (send_s(socket_array[i], &closedFlag, sizeof(closedFlag), 0)) return 1;
        close(socket_array[i]);
    }

    // Close the socket
    close(socket_fd);
    return 0;
}
