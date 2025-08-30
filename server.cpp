#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <assert.h>
#include <thread> //thread for multiple clients to get connected at a time

using namespace std;

// for error

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
        int rv = read(fd, rbuff, n);
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
        u_int32_t rv = write(fd, msg, n);
        if (rv <= 0)
            return -1; // error
        assert(rv <= n);
        n -= rv;
        msg += rv;
    }
    return 0;
}

static u_int32_t k_max_len = 4096; // for the max length fothe message

static int one_request(int connfd)
{
    char rbuff[4 + k_max_len];
    errno = 0;
    int err = read_full(connfd, rbuff, 4);
    if (err != 0)
        return -1;
    u_int32_t len;
    memcpy(&len, rbuff, 4);
    if (len > k_max_len)
    {
        msg("too long ");
        return -1;
    }
    err = read_full(connfd, rbuff + 4, len);
    if (err)
    {
        msg("read() error ");
        return -1;
    }

    // printing msg stored

    string message(rbuff + 4, len);
    cout << "Client says " << message << endl;

    // writing something

    const string ack = "recieved";
    len = (u_int32_t)ack.length();
    char w_buff[4 + len];
    memcpy(w_buff, &len, 4);
    memcpy(w_buff + 4, ack.data(), len);
    return write_all(connfd, w_buff, len + 4);
}

void client_handler(int conn_fd)
{
    while (true)
    {
        int err = one_request(conn_fd);
        if (err)
            break;
    }
    close(conn_fd);
}

int main()
{
    // Obtain socket handle
    int fd = socket(AF_INET, SOCK_STREAM, 0);
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

    // accept connections

    while (true)
    {
        struct sockaddr_in client_addr = {};
        socklen_t addlen = sizeof(client_addr);
        int conn_fd = accept(fd, (struct sockaddr *)&client_addr, &addlen);
        if (conn_fd < 0)
            continue;
        thread(client_handler, conn_fd).detach();
    }
    close(fd);
    return 0;
}