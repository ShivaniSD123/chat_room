# chat_room
<h2>Description</h2>
<p> A multi-threaded chatroom application implemented in C++, with user authentication (register/login) backed by a relational database (MySQL).

</p>

<h2>Features</h2>
<p>
<ul>
<li>Client-Server Architecture- Central server manages all the clients</li>
<li>Multi-user Chat: Multiple clients can connect and chat simultaneously</li>
<li>Console-based messaging for both server and client</li>
<li>User Authentication- Register an new user and login the existing user.</li>
<li>Both clients and server can exit gracefully with the command quit or Quit.</li>
<li>Message Broadcasting- Messages sent with username tags, not client IDs.</li>
<li>Concurrency: Each client runs on a dedicated thread.</li>
</ul>
</p>
<p>
<h2>Requirements</h2>
<ul>
<li>C++11 or later</li>
<li>MySQL Database for authentication</li>

<li>CMake for build system</li>

<li>Linux/Unix environment (uses sys/socket.h, netinet/in.h>, etc.)</li>

<li>Basic build tools (g++)</li>
</ul>
</p>

<p>
<h2> Installations and Stepus</h2>
<h4>Configure Database</h4>
<p>Create a MySQL database named chatroom. And add users table for authentication:</p>
<p>
CREATE TABLE users (<br>
    id INT AUTO_INCREMENT PRIMARY KEY,<br>
    username VARCHAR(50) UNIQUE NOT NULL,<br>
    password VARCHAR(255) NOT NULL<br>
);<br>
</p>
<li>Update DB credentials in authenticate.h (or config file) (eg. user and password in authenticate.h).</li>
<h4>NOTE: THE DATABSE AND THE SERVER SHOULD BE ON SAME MACHINE</h4>
<h4>Build the project</h4>
<p>cmake.<br>
make<br></p>
<h4> Running the Server</h4>
<p>./server</p>
<h4>Running the client</h4>
<p>./client</p>
<p>
<h2>Future Improvements</h2>
<ul>
<li>
Replace per-thread model with event-driven I/O. </li>

<li>Add encryption for secure communication.</li>

<li>Store and retrieve chat history from the database</li>

<li>Add private messaging (/msg user).</li>

<li>Improve error handling and reconnection support.</li>
</ul>
</p>
