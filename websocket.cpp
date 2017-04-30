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
#include <vector>

std::string mask_message(std::string message, char* mask){
    std::string out = "";
    for(int i = 0 ; i < message.size(); ++i)
	out += char(message[i] ^ mask[i % 4]);
    return out;
}

std::string generate_accept(std::string key){
    key.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    unsigned char const* hash = SHA1(reinterpret_cast<const unsigned char*>(key.c_str()), key.length(), nullptr);
    return base64_encode(hash, 20);
}

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
	
	if(ws){ // if websocket handshake
	    std::string key = buf.substr(buf.find("Sec-WebSocket-Key") + 19,
				buf.substr(buf.find("Sec-WebSocket-Key")).find("\n") - 20);
	    std::string key_accept = generate_accept(key);

	    reply =
		"HTTP/1.1 101 Switching Protocols\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Accept: " + key_accept + "\r\n\r\n";
	    
	}else{ // non-handshake, interpreted as "normal" request
	    reply =
		"HTTP/1.1 200 OK\r\n"
		"\n<script>var ws = new WebSocket('ws://192.168.10.117:8080');\n"
		"ws.addEventListener('open',function(event){"
		"\n\tconsole.log('open!!');\n\tws.send('yo bro!');\n});\n"
		"var sendmessage = function(){console.log('click');ws.send('test123567890"
		"A1234567890B1234567890C1234567890D1234567890E1234567890F1234567890"
		"F1234567890G1234567890H1234567890I1234567890J1234567890K1234567890"
		"L1234567890M1234567890N1234567890O1234567890P1234567890Q1234567890');};"
		"ws.onmessage = function(event){console.log(event.data);};</script>"
		"<input type=\"text\"><button onclick=\"sendmessage()\">send</button>\r\n";
	    // hardcoded websocket testing interface
	}
	send(connfd, reply.c_str(), reply.size(), 0);
	
	std::cout << "sent " << (ws ? " socket accept" : "webpage") << " to " << connfd << " (" << address<<")" << std::endl;

	if(ws){
	    std::cout << "starting new thread for " << connfd << " (" << address << ")"<< std::endl;
	    std::thread thr([&connfd]() {
		    int cpfd = connfd;
		    std::vector<char*> segments;
		    while(true){
			char wsbuff[1400];
			int fsize = read(cpfd, wsbuff, 1400);
			if(fsize > 0){
			    uint64_t size = char(wsbuff[1] ^ 0x80);
			    int offset = 6; // payload start;
			    if(size == 126){ // size class 2 ( < 65536) (buffer is too small)
				size = (((unsigned int)wsbuff[2] << 8) | (unsigned char)wsbuff[3]);
				offset = 8;
			    }else if(size == 127){ // size class 3 (huge)
			    	size = 0;
				for(int i = 0; i < 8; ++i)
				    size |= (uint64_t(wsbuff[2 + i]) << 8 * (7 - i));
				offset = 14;
			    }
			    std::cout << "received " << fsize << " bytes, offset = " << offset << " size = " << size << std::endl;
			    char mask[4] = {wsbuff[offset - 4], wsbuff[offset - 3],
					    wsbuff[offset - 2], wsbuff[offset - 1]};
			    std::string message = "";
			    
			    for(int i = 0; i < size; ++i)
				message.push_back(char(wsbuff[offset + i] ^ mask[i % 4]));
			    std::cout << "message from client: " << message << std::endl;

			    // send back all but the mask (4 bytes)
			    char* response = (char*)malloc(sizeof(char) * (fsize - 4));
			    
			    for(int i = 0; i < offset - 4; ++i)
				response[i] = wsbuff[i];
			    for(int i = 0; i < size; ++i)
				response[offset - 4 + i] = message[i];
			    response[1] ^= 0x80;
			    send(cpfd, response, fsize - 4, 0);
			}
			usleep(1000);
		    }
		});
	    thr.detach();
	    ws = false;
	}
	//close(connfd);
        usleep(1000); // to save the CPU
     }
}

