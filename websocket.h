#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <string>
#include <iostream>
#include <openssl/sha.h>
#include "include/base64.h"
#include <bitset>
#include <thread>
#include <mutex>
#include <vector>

namespace ws{
    struct wsmsg{
        int sock_id;
        std::string message;
    };

    class websocket {
    private:
        int listenfd;
        int connfd;
	struct sockaddr_in serv_addr;
	struct sockaddr caddr;
        std::string webpage;
        std::vector<int> clients;
        std::vector<wsmsg> messages;
        std::mutex clients_mx;
        std::mutex messages_mx;

        std::string generate_accept(std::string key);
        std::string mask_message(std::string message, char* mask);
	
        void new_websocket(int connfd);
        void run();
    public:
        void start(int port, std::string wp);
	int send_message(int id, std::string message);
	int broadcast_message(std::string message);
        std::vector<wsmsg>  get_msg_buffer();
        std::vector<int> get_clients();
    };
}

