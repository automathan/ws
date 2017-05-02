#include "websocket.h"

int main(int argc, char *argv[]){
    std::string serverip = "192.168.10.117";
    std::string webpage =
        "HTTP/1.1 200 OK\r\n\n<html>"
	"<link rel=\"icon\" href=\"data:;base64,=\">"
	"<canvas id=\"canvas\" width=\"256\" height=\"256\"></canvas>"
	"<script>"
	"var ws = new WebSocket('ws://" + serverip + ":8080');\n"
        "ws.addEventListener('open',function(event){"
        "\n\tconsole.log('open!!');});\n\t"
        "ws.onmessage = function(event){"
	"console.log(event.data);"
	"document.getElementById('canvas').getContext('2d').clearRect(0,0,256,256);"
	"document.getElementById('canvas').getContext('2d').font = '12px Arial';"
	"document.getElementById('canvas').getContext('2d').fillStyle = '#000000';"
	"var lines = event.data.split('\\n');"
	"for(var i = 1; i < lines.length; ++i){"
	"var values = lines[i].split(',');"
	"console.log(values, values.length);"
	"if(values.length == 3){"
	"console.log('a');"
	"document.getElementById('canvas').getContext('2d').fillText(values[0], values[1], values[2]);"
	"console.log('b');"
	"}}"
	"};"
        "ws.onclose = function(){console.log('closed succesfully!');};"
        "document.onkeydown = function(event){"
        "if(event.keyCode === 65)"
        "    ws.send('l');"
        "if(event.keyCode === 68)"
        "    ws.send('r');"
        "if(event.keyCode === 87)"
        "    ws.send('u');"
        "if(event.keyCode === 83)"
        "    ws.send('d');"
	"};"
	"</script></html>";

    ws::websocket ws;
    ws.start(8080, webpage);

    struct player{
	int id;
	int x;
	int y;
    };
    std::vector<player> players;
    while(true){
	std::vector<ws::wsmsg> messages = ws.get_msg_buffer();
	for(int i = 0; i < messages.size(); ++i){
	    bool pfound = false;
	    for(int j = 0; j < players.size(); ++j){
		if(players[j].id == messages[i].sock_id){
		    pfound = true;
		    if(strcmp(messages[i].message.c_str(), "l"))
			players[j].x += 16;
		    if(strcmp(messages[i].message.c_str(), "r"))
			players[j].x -= 16;
		    if(strcmp(messages[i].message.c_str(), "u"))
			players[j].y += 16;
		    if(strcmp(messages[i].message.c_str(), "d"))
			players[j].y -= 16;
		}
	    }
	    if(!pfound){
		players.push_back({messages[i].sock_id, 128, 128});
	    }
	}
	std::string gameinfo = "gameinfo\n";
	for(int i = 0; i < players.size(); ++i){
	    gameinfo += std::to_string(players[i].id) + ",";
	    gameinfo += std::to_string(players[i].x) + ",";
	    gameinfo += std::to_string(players[i].y) + "\n";
	    }
	ws.broadcast_message(gameinfo);
	usleep(100000);
    }
}
