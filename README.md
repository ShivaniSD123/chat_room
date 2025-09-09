# chat_room
<h2>Description</h2>
<p> This is a simple unix based soft real time TCP based chat room application written in c++
<ul>
<li>The server can handle multiple clients simultaneously using threads.</li>

<li>Messages from one client are broadcast to all other connected clients.</li>

<li>The server can also send messages to all clients.</li>

<li>Both clients and server can exit gracefully with the command quit or Quit.</li>
</ul>
</p>

<h2>Features</h2>
<p>
<ul>
<li>Client-Server Architecture- Central server manages all the clients</li>
<li>Multi-user Chat: Multiple clients can connect and chat simultaneously</li>
<li>Console-based messaging for both server and client</li>
</ul>
</p>
<p>
<h2>Requirements</h2>
<ul>
<li>C++11 or later</li>

<li>Linux/Unix environment (uses <sys/socket.h>, <netinet/in.h>, etc.)</li>

<li>Basic build tools (g++)</li>
</ul>
</p>
<p>
<h2>Future Improvements</h2>
<ul>
<li>
Support for usernames instead of socket IDs.</li>

<li>Private messaging between clients.</li>

<li>GUI front-end (Qt, ncurses, or web sockets).</li>

<li>Better client ID management (instead of raw socket numbers).</li>
</ul>
</p>
