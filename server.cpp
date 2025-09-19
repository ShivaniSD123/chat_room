#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <assert.h>
#include <thread> //thread for multiple clients to get connected at a time
#include <mutex>
#include "authenticate.h"

using namespace std;

// for error
vector<int> clients;
mutex gLock;
static int fd;
void die(string msg)
{
    int err = errno;
    cerr << "[" << err << "] " << "(" << msg << ") " << strerror(err) << endl;
    abort();
}

void msg(string str)
{
    cerr << str << endl;
}

// read_full and write_all function for reading and writing multiple bytes at a time

static int read_full(int fd, char *rbuff, u_int32_t n)
{
    while (n > 0)
    {
        ssize_t rv = read(fd, rbuff, n);
        if (rv <= 0)
        {
            return -1; // error
        }
        assert(u_int32_t(rv) <= n);
        n -= (u_int32_t)rv;
        rbuff += rv;
    }
    return 0;
}

static int write_all(int fd, const char *msg, u_int32_t n)
{
    while (n > 0)
    {
        ssize_t rv = write(fd, msg, n);
        if (rv <= 0)
            return -1; // error
        assert((u_int32_t)rv <= n);
        n -= (u_int32_t)rv;
        msg += rv;
    }
    return 0;
}

static u_int32_t k_max_len = 4096; // for the max length fothe message

// broadcast message

void broadcast(const char *message, int sender_id = -1)
{ // default value of sender_id =-1
    lock_guard<mutex> lg(gLock);
    u_int32_t len = strlen(message);
    char w_buff[8 + len];
    memcpy(w_buff, &len, 4);
    memcpy(w_buff + 4, &sender_id, 4);
    memcpy(w_buff + 8, message, len);
    for (int client_id : clients)
    {
        if (client_id != sender_id)
        {

            int err = write_all(client_id, w_buff, len + 8);
        }
    }
}

// client_handler- Display ans broadcast the message
void client_handler(int conn_fd)
{
    while (true)
    {
        char r_buff[4 + k_max_len];
        int err = read_full(conn_fd, r_buff, 4);
        if (err != 0)
        {
            msg("read error()");
            return;
        }
        u_int32_t len;
        memcpy(&len, r_buff, 4);
        err = read_full(conn_fd, r_buff + 4, len);
        if (err != 0)
        {
            msg("read error()");
            return;
        }
        string mess(r_buff + 4, len);
        string temp = mess;
        if (mess == "Quit" || mess == "quit")
        {
            lock_guard<mutex> lg(gLock);
            for (auto itr = clients.begin(); itr != clients.end(); itr++)
            {
                if (*itr == conn_fd)
                {
                    clients.erase(itr);
                    mess = "Client " + std::to_string(conn_fd) + " exited";
                    break;
                }
            }
        }
        broadcast(mess.c_str(), conn_fd);
        cout << "[Client " << conn_fd << "] says- " << mess << endl;
        if (temp == "Quit" || temp == "quit")
        {
            close(conn_fd);
            return;
        }
    }
}

// server handler- for input and broadcast message by server

void server_handler()
{
    string server_msg;
    while (getline(cin, server_msg))
    {
        if (server_msg == "Quit" || server_msg == "quit")
        {
            cout << "Server Shutting down " << endl;
            broadcast("Server shutting down", -1);
            close(fd);
            exit(0);
            return;
        }
        broadcast(server_msg.c_str(), -1);
    }
}

// authenticate fn

bool chk(int conn_fd)
{
    char r_buff[4 + k_max_len];
    int err = read_full(conn_fd, r_buff, 4);
    if (err != 0)
    {
        msg("read error()");
        return 0;
    }
    u_int32_t name_len;
    memcpy(&name_len, r_buff, 4);
    err = read_full(conn_fd, r_buff + 4, name_len);
    string name(r_buff + 4, name_len);
    char choice = name[0];
    string username = name.substr(1, name_len - 1);
    u_int32_t pass_len;
    err = read_full(conn_fd, r_buff + 4 + name_len, 4);
    memcpy(&pass_len, r_buff + 4 + name_len, 4);
    err = read_full(conn_fd, r_buff + 8 + name_len, pass_len);
    string pass(r_buff + 8 + name_len, pass_len);
    cout << choice << " Username " << username << "  password- " << pass << endl;
    bool is_valid = 0;
    if (choice == '0')
        is_valid = reg(username, pass);
    else
        is_valid = login(username, pass);
    char w_buff[4];
    int i = is_valid;
    memcpy(w_buff, &i, 4);
    err = write_all(conn_fd, w_buff, 4);
    return is_valid;
}
int main()
{
    // Obtain socket handle
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        die("socket()");

    // set Socket option-
    // As we stop server, it can be reused immidiately if want

    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // defing and binding the socket

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET; // for IPv4
    addr.sin_addr.s_addr = htonl(0);
    addr.sin_port = htons(1234);
    int rv = ::bind(fd, (const struct sockaddr *)&addr, sizeof(addr));

    // listen

    rv = listen(fd, SOMAXCONN); // SOMAXCONN-> user defind constant (for maximum reasonable length of pending connections in queue)
    if (rv)
        die("listen()");
    thread(server_handler).detach();

    // accept connections

    while (true)
    {
        struct sockaddr_in client_addr = {};
        socklen_t addlen = sizeof(client_addr);
        int conn_fd = accept(fd, (struct sockaddr *)&client_addr, &addlen);
        if (conn_fd < 0)
            continue;
        if (chk(conn_fd))
        {
            {
                lock_guard<mutex> lg(gLock);
                clients.push_back(conn_fd);
            }

            thread(client_handler, conn_fd).detach();
            string j_msg = "Client " + to_string(conn_fd) + " joined chat";
            cout << j_msg << endl;
            broadcast(j_msg.c_str(), conn_fd);
        }
        else
        {
            close(conn_fd);
        }
    }
    close(fd);
    return 0;
}