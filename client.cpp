#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>   //for error number
#include <cstdlib>  //for abort
#include <cstring>  //for strerror
#include <unistd.h> // for write
#include <assert.h> //for assert
#include <thread>   //thead used for broadcasting
#include <mutex>

using namespace std;

static void msg(string str)
{
    cerr << str << endl;
}

void die(string msg)
{
    int err = errno;
    cerr << "[" << err << "] " << msg << " (" << strerror(err) << ")" << endl;
    abort();
}

static int read_full(int fd, char *rbuff, u_int32_t n)
{
    while (n > 0)
    {
        u_int32_t rv = read(fd, rbuff, n);
        if (rv == 0)
        {
            // msg("EOF");
            return -1;
        }
        else if (rv < 0)
        {
            msg("read() error ");
            return -1;
        }
        assert(rv <= n);
        n -= rv;
        rbuff += rv;
    }
    return 0;
}

static int write_all(int fd, char *w_buff, u_int32_t n)
{
    while (n > 0)
    {
        int rv = write(fd, w_buff, n);
        if (rv <= 0)
            return -1;
        assert(rv <= n);
        n -= rv;
        w_buff += rv;
    }
    return 0;
}

const u_int32_t k_max_size = 4096;

static int recieve_handler(int fd)
{
    bool running = true;
    while (running)
    {

        char r_buff[8 + k_max_size];
        errno = 0;
        int err;
        err = read_full(fd, r_buff, 4);
        if (err != 0)
        {
            msg("Server disconnected");
            running = false;
            return -1;
        }
        u_int32_t len;
        memcpy(&len, r_buff, 4);
        if (len > k_max_size)
        {
            msg("Too long message");
            running = false;
        }
        err = read_full(fd, r_buff + 4, 4);
        u_int32_t client_no;
        memcpy(&client_no, r_buff + 4, 4);
        err = read_full(fd, r_buff + 8, len);
        string message(r_buff + 8, len);
        if (client_no > 4096)
            cout << "[Server] " << message << endl;
        else
            cout << "[Client " << client_no << "] " << message << endl;
    }

    return 0;
}

static int send_handler(int fd)
{
    string message;
    cout << "Type the message " << endl;
    bool running = true;
    while (running)
    {

        getline(cin, message);
        char w_buff[4 + message.length()];
        if (message == "Quit" || message == "quit")
        {
            running = false;
            u_int32_t len = (u_int32_t)message.length();
            memcpy(w_buff, &len, 4);
            memcpy(w_buff + 4, message.c_str(), len);
            int err = write_all(fd, w_buff, len + 4);
            if (err != 0)
            {
                msg("Server disconnected()");
                running = false;
                return -1;
            }
            break;
        }
        if (message.size() > k_max_size)
        {
            running = false;
            msg("message too long() ");
            continue;
        }
        u_int32_t len = (u_int32_t)message.length();
        memcpy(w_buff, &len, 4);
        memcpy(w_buff + 4, message.c_str(), len);
        int err = write_all(fd, w_buff, len + 4);
        if (err != 0)
        {
            msg("Server disconnected()");
            running = false;
            return -1;
        }
    }
    return 0;
}

int main()
{
    // Obtaining socket handler
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        die("socket()");

    // Defining The Client Address and binding

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(1234);

    // Establishing the Connection

    int rv = connect(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv)
        die("connect() ");

    // Read and Write multiple requests for test
    cout << "Connected to chat room" << endl;
    cout << "Type Quit or quit to exit" << endl;
    thread r_thread(&recieve_handler, fd);
    thread s_thread(&send_handler, fd);
    r_thread.join();
    s_thread.join();
    close(fd);
    return 0;
}