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

string name;
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
const u_int32_t k_max_size = 4096;

static int recieve_handler(int fd)
{
    bool running = true;
    while (running)
    {
        char r_buff[4 + k_max_size];
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
        err = read_full(fd, r_buff + 4, len);
        string message(r_buff + 4, len);
        cout << message << endl;
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
        char w_buff[8 + message.length() + name.length()];
        if (message.size() > k_max_size)
        {
            running = false;
            msg("message too long() ");
            continue;
        }
        u_int32_t name_len = name.length();
        u_int32_t len = (u_int32_t)message.length();
        memcpy(w_buff, &name_len, 4);
        memcpy(w_buff + 4, name.c_str(), name_len);
        memcpy(w_buff + 4 + name_len, &len, 4);
        memcpy(w_buff + 8 + name_len, message.c_str(), len);
        int err = write_all(fd, w_buff, len + 8 + name_len);
        if (err != 0)
        {
            msg("Server disconnected()");
            running = false;
            return -1;
        }
        if (message == "Quit" || message == "quit")
            running = false;
    }
    close(fd);
    return 0;
}

// register and login

bool chk(int fd)
{

    string pass;
    string option;
    cout << "\t\tWelcome to Chat-Box" << endl;
    cout << "===================================================" << endl
         << endl;
    cout << "Choose 0 to Register and 1 to Login, others to exit" << endl;
    getline(cin, option);
    if (option != "0" && option != "1")
        return 0;
    cout << "Enter the Username ";
    getline(cin, name);
    cout << "Enter the Password ";
    getline(cin, pass);
    name = option + name;
    u_int32_t name_len = name.size();
    u_int32_t pass_len = pass.size();
    char w_buff[8 + name_len + pass_len];
    memcpy(w_buff, &name_len, 4);
    memcpy(w_buff + 4, name.c_str(), name_len);
    memcpy(w_buff + 4 + name_len, &pass_len, 4);
    memcpy(w_buff + 8 + name_len, pass.c_str(), pass_len);
    int err = write_all(fd, w_buff, 8 + name_len + pass_len);
    char r_buff[4];
    err = read_full(fd, r_buff, 4);
    u_int32_t reply;
    memcpy(&reply, r_buff, 4);
    if (reply == 0)
        cout << "Rejected" << endl;
    else
        cout << "Authenticated" << endl;
    name = name.substr(1, name_len - 1);
    return bool(reply);
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
    if (chk(fd))
    {

        // Read and Write multiple requests for test
        cout << "Connected to chat room" << endl;
        cout << "Type Quit or quit to exit" << endl;
        thread r_thread(&recieve_handler, fd);
        thread s_thread(&send_handler, fd);
        r_thread.join();
        s_thread.join();
    }
    else
    {
        cout << "Wrong Information" << endl;
    }
    close(fd);
    return 0;
}