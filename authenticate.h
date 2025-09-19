#include <iostream>
#include </usr/local/mysql/include/mysql.h>
#include <string>

// MYSQL *conn = mysql_init(NULL);

bool reg(std::string name, std::string pass)
{
    MYSQL *conn = mysql_init(NULL);
    if (!conn)
    {
        std::cerr << "Error";
        return 1;
    }
    if (mysql_real_connect(conn, "127.0.0.1", "root", "jaishreeram108", "chatroom", 3306, NULL, 0) == NULL)
    {
        mysql_close(conn);
        return EXIT_FAILURE;
    }
    std::string query = "insert into users (username, password_hash) values ('" + name + "', '" + pass + "')";
    if (mysql_query(conn, query.c_str()))
    {
        mysql_close(conn);
        return false;
    }
    mysql_close(conn);
    return true;
}

bool login(std::string name, std::string pass)
{
    MYSQL *conn = mysql_init(NULL);
    if (!conn)
    {
        std::cerr << "Error";
        return 1;
    }
    if (mysql_real_connect(conn, "127.0.0.1", "root", "jaishreeram108", "chatroom", 3306, NULL, 0) == NULL)
    {
        mysql_close(conn);
        return EXIT_FAILURE;
    }
    std::string query = "select* from users where username= '" + name + "' and password_hash= '" + pass + "'";
    mysql_query(conn, query.c_str());
    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row;
    if ((row = mysql_fetch_row(res)) == NULL)
    {
        mysql_free_result(res);
        return false;
    }
    mysql_free_result(res);
    mysql_close(conn);
    return true;
}
