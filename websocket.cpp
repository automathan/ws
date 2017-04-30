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
//cpp
#include <string>
#include <iostream>
#include <openssl/sha.h>
#include "include/base64.h"
#include <bitset>
#include <thread>

int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 
    struct sockaddr caddr;
    
    time_t ticks; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(8080); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(listenfd, 8080);
    bool ws = false;
    while(1)
    {
	connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
	socklen_t ll = 14;
	getpeername(connfd, &caddr, &ll);
	std::string address(caddr.sa_data);
	std::cout << "connfd = " << connfd << std::endl;

	char buffer[1400];
	
	read(connfd, buffer, 1400);

	std::string buf(buffer);
	std::cout << buf << std::endl;
	
	std::string reply;
	
	ws = strstr(buffer, "Upgrade: websocket");
	if(ws){ // if websocket handshake. This works
	    std::cout << "<websocket>" << std::endl;
	    std::string key = buf.substr(buf.find("Sec-WebSocket-Key") + 19,
				buf.substr(buf.find("Sec-WebSocket-Key")).find("\n") - 20);
	    
	    std::cout << "key = " << key << std::endl;
	    key.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	    std::cout << "key = " << key << " length = " << key.length() << std::endl;
	    unsigned char const* hash = SHA1(reinterpret_cast<const unsigned char*>(key.c_str()), key.length(), nullptr);
	    std::string b64 = base64_encode(hash, 20);
	    std::cout << "b64 = " << b64 << std::endl;
	    
	    reply =
		"HTTP/1.1 101 Switching Protocols\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Accept: " + b64 + "\r\n\r\n";
	}else{
	    std::cout << "<other>" << std::endl;
	    reply =
		"HTTP/1.1 200 OK\r\n"
		"\n<script>var ws = new WebSocket('ws://192.168.10.117:8080');\n"
		"ws.addEventListener('open',function(event){"
		"\n\tconsole.log('open!!');\n\tws.send('yo bro!');\n});\n"
		"var sendmessage = function(){console.log('click');ws.send('test123567890');};"
		"ws.onmessage = function(event){console.log(event.data);};</script>"
		"<input type=\"text\"><button onclick=\"sendmessage()\">send</button>\r\n";
	}
	send(connfd, reply.c_str(), reply.size(), 0);
	std::cout << "sent " << (ws ? " socket accept" : "webpage") << " to " << connfd << " (" << address<<")" << std::endl;
	if(ws){
	    std::cout << "starting new thread for " << connfd << " (" << address << ")"<< std::endl;
	    std::thread thr([&connfd]() {
		    int cpfd = connfd;
		    while(true){
			char wsbuff[1400];
			int abc = read(cpfd, wsbuff, 1400);
			if(abc > 0){
			    std::cout << "received " << abc << " bytes:" << std::endl;
			    char mask[4] = {wsbuff[2], wsbuff[3], wsbuff[4], wsbuff[5]};
			    for(int i = 6; i < abc; ++i)
				std::cout << char(wsbuff[i] ^ mask[(i - 2) % 4]);
			    std::cout << std::endl;
			    char* response = (char*)malloc(sizeof(char) * (abc - 4));
			    response[0] = 0x81;
			    response[1] = char(abc - 6);
			    for(int i = 6; i < abc; ++i)
				response[i - 4] = wsbuff[i] ^ mask[(i - 2) % 4];
			    send(cpfd, response, abc - 4, 0);
			}
			usleep(10);
		    }
		});
	    thr.detach();
	    ws = false;
	}
	//close(connfd);
        usleep(10);
     }
}
