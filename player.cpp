#include "potato.h"

int main(int argc, char *argv[]) {
    // Check for cli arguments
    if (check_cli_argv(argc, 3)) return 1;

    const char *hostname = argv[1];
    const char *port = argv[2];
    int socket_server;
    int socket_listen;
    int socket_left;
    int socket_right;
    int id;
    int num_players;

    // Establish connection with error checking
    if (establish_connection(&socket_server, port, true, hostname)) return 1;

    // Tell server the client is ready
    int connectedFlag = 1;
    if (send_s(socket_server, &connectedFlag, sizeof(connectedFlag), 0)) return 1;

    // Receive assigned id and total player number
    int received_message[2];
    assert(recv(socket_server, received_message, sizeof(received_message), MSG_WAITALL) == sizeof(received_message));
    id = received_message[0];
    num_players = received_message[1];

    std::cout << "Connected as player " << id << " out of " << num_players << " total players" << std::endl;

    // Connect with neighbors
    while (1) {
        int signal;
        assert(recv(socket_server, &signal, sizeof(signal), MSG_WAITALL) == sizeof(signal));
        std::cout << signal << std::endl;
        // All players are ready
        if (signal == 0) break;
        else if (signal == 1) {
            // Listen to left socket
            char port_to_listen[48];
            convert_port(id, num_players, port_to_listen);

            // Establish binding with error checking
            if (establish_connection(&socket_listen, port_to_listen, false, NULL, true)) return 1;
            // Check listen status
            if (check_status(listen(socket_listen, 100), -1, 3)) return 1;

            // Accept socket left
            sockaddr_t socket_addr_left;
            socklen_t socket_addr_len_left = sizeof(socket_addr_left);
            if (check_status(socket_left = accept(socket_listen, (struct sockaddr *) &socket_addr_left, &socket_addr_len_left), 1,
                             4))
                return 1;

            connectedFlag = 0;
            assert(recv(socket_left, &connectedFlag, sizeof(connectedFlag), MSG_WAITALL) == sizeof(connectedFlag));
            if (connectedFlag) {
                if (send_s(socket_server, &connectedFlag, sizeof(connectedFlag), 0)) return 1;
            } else return 1;

        } else if (signal == 2) {
            // Create connection to right client
            sockaddr_t socket_addr_right;
            socklen_t socket_addr_len_right = sizeof(socket_addr_len_right);
            assert(recv(socket_server, &socket_addr_len_right, sizeof(socket_addr_len_right), MSG_WAITALL) ==
                   sizeof(socket_addr_len_right));
            assert(recv(socket_server, &socket_addr_right, socket_addr_len_right, MSG_WAITALL) ==
                   socket_addr_len_right);
            char port_to_connect_str[48];
            convert_port(id, num_players, port_to_connect_str, 1);

            struct sockaddr_in *temp = (struct sockaddr_in *) &socket_addr_right;
            std::cout << id << " " << port_to_connect_str << std::endl;

            if (establish_connection(&socket_right, port_to_connect_str, true, inet_ntoa(temp->sin_addr), false,
                                     true))
                return 1;

            connectedFlag = 1;
            if (send_s(socket_right, &connectedFlag, sizeof(connectedFlag), 0)) return 1;
        }
    }

    // Start Game
    int is_game_over = false;
    fd_set rfds;
    int recval;
    potato_t potato;
    memset(&potato, 0, sizeof(potato));
    int max_fdid;
    // Find max fdid
    max_fdid = (socket_left > socket_right) ? socket_left : socket_right;
    max_fdid = (socket_server > max_fdid) ? socket_server : max_fdid;

    srand((unsigned int) time(NULL) + id);

    // Game loop
    while (!is_game_over) {
        // Clear fd set
        FD_ZERO(&rfds);
        // Set fd set
        FD_SET(socket_server, &rfds);
        FD_SET(socket_left, &rfds);
        FD_SET(socket_right, &rfds);

        recval = select(max_fdid + 1, &rfds, NULL, NULL, NULL);
        if (check_status(recval, -1, 8)) return 1;

        // Receive potato
        if (FD_ISSET(socket_server, &rfds))
            assert(recv(socket_server, &potato, sizeof(potato), MSG_WAITALL) == sizeof(potato));

        if (FD_ISSET(socket_left, &rfds))
            assert(recv(socket_left, &potato, sizeof(potato), MSG_WAITALL) == sizeof(potato));

        if (FD_ISSET(socket_right, &rfds))
            assert(recv(socket_right, &potato, sizeof(potato), MSG_WAITALL) == sizeof(potato));

        // Check if game is over
        if (potato.is_over)
            is_game_over = true;
        else {
            // Pass potato
            potato.num_hops -= 1;
            potato.current_num += 1;
            potato.traces[potato.current_num] = id;

            if (potato.num_hops == 0) {
                std::cout << "I'm it" << std::endl;
                if (send_s(socket_server, &potato, sizeof(potato), 0)) return 1;
            } else {
                int rand_send = rand() % 2;
                if (rand_send == 0) {
                    if (id == num_players - 1)
                        std::cout << "Sending potato to " << 0 << std::endl;
                    else
                        std::cout << "Sending potato to " << id + 1 << std::endl;
                    if (send_s(socket_left, &potato, sizeof(potato), 0)) return 1;
                } else {
                    if (id == 0)
                        std::cout << "Sending potato to " << num_players - 1 << std::endl;
                    else
                        std::cout << "Sending potato to " << id - 1 << std::endl;
                    if (send_s(socket_right, &potato, sizeof(potato), 0)) return 1;
                }
            }
        }
    }

    FD_ZERO(&rfds);

    // Send game over flag to server
    int gameOverFlag = 1;
    if (send_s(socket_server, &gameOverFlag, sizeof(gameOverFlag), 0)) return 1;

    // Receive close instruction from server
    int closedFlag = 0;
    assert(recv(socket_server, &closedFlag, sizeof(closedFlag), MSG_WAITALL) == sizeof(closedFlag));
    assert(closedFlag == 1);

    // Close sockets
    close(socket_left);
    close(socket_right);
    close(socket_listen);
    close(socket_server);

    return 0;
}
