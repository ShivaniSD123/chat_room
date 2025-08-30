#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>   //for error number
#include <cstdlib>  //for abort
#include <cstring>  //for strerror
#include <unistd.h> // for write
#include <assert.h> //for assert

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

static int query(int fd, const char *text)
{
    u_int32_t len = (u_int32_t)strlen(text);
    if (len > k_max_size)
        return -1;
    char w_buff[4 + len];
    memcpy(w_buff, &len, 4);
    memcpy(w_buff + 4, text, len);
    int err = write_all(fd, w_buff, len + 4);
    if (err)
        return err;

    char rbuff[4 + k_max_size];
    errno = 0;
    err = read_full(fd, rbuff, 4);
    if (err != 0)
    {
        msg("read() error ");
        return -1;
    }

    memcpy(&len, rbuff, 4);
    if (len > k_max_size)
        msg("too long ");
    err = read_full(fd, rbuff + 4, len);
    if (err)
        msg("read() error ");

    // printing msg stored

    string message(rbuff + 4, len);
    cout << "Server says " << message << endl;
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
    string message;
    cout << "Enter the message";
    std::getline(cin, message);
    while (message != "Quit" && message != "quit")
    {
        int err = query(fd, message.c_str());
        if (err)
        {
            close(fd);
            return 0;
        }
        cout << "Enter the message";
        std::getline(cin, message);
    }
    close(fd);
    return 0;
}