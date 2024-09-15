#include <iostream>
#include "MysqlDispatcher.h"

int query()
{
    MysqlDispatcher conn;
    conn.connect("root", "123", "CrossTime", "192.168.10.132");
    std::string sql = "insert into vip VALUES(3,0,0,0,\'2024-04-22 19:35:16\',0,0,0);";
    bool ret = conn.update(sql);
    std::cout << "ret =  " << ret << std::endl;

    sql = "select * from vip;";
    conn.query(sql);
    while (conn.next())
    {
        std::cout << conn.value(0) << ","
                  << conn.value(1) << ","
                  << conn.value(2) << ","
                  << conn.value(3) << ","
                  << conn.value(4) << ","
                  << conn.value(5) << ","
                  << conn.value(6) << ","
                  << conn.value(7) << std::endl;
    }
    return 0;
}

int main()
{
    query();
    return 0;
}