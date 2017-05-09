This is a very simple websocket implementation in C/C++<br>

Functionality:
- Handshake
- Support for multiple clients
- Supports large messages
- Can broadcast a message to all active clients
- Can send a message to a specific client by id
- Can list all live client ids
- Can give you a buffer of all messages revieved
- Host a basic chat-server with ~10 lines of code :)

Check example.cpp for a very basic client using all the functionalities<br>
How to compile the example:<br>
g++ example.cpp websocket.cpp include/base64.cpp -lcrypto -pthread<br>


Confirmed to run well on Manjaro Linux<br>
For some reason it takes a while to connect a client to the server, but it works.<br>

Base64 source:
http://renenyffenegger.ch/notes/development/Base64/Encoding-and-decoding-base-64-with-cpp
**Sources of information:**<br>
https://developer.mozilla.org/en-US/docs/Web/API/WebSocket<br>
Incredibly well written source for websocket implementation<br>
https://tools.ietf.org/html/rfc6455<br>
Detailed descriptions of everything<br>
