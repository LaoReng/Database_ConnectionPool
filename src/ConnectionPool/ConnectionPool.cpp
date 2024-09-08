#include "ConnectionPool.h"
#include "json.h"
#include <fstream>

ConnectionPool::ConnectionPool()
{

}

ConnectionPool *ConnectionPool::getConnectPool()
{
    // 在c++11里面静态局部变量是线程安全的
    static ConnectionPool pool;
    return &pool;
}

bool ConnectionPool::parseJsonFile()
{
    std::ifstream ifs("config/dbconf.json");
    Json::Reader rd;
    Json::Value root;
    rd.parse(ifs, root);
    // 判断数据是否为json对象
    if (root.isObject())
    {
        m_ip = root["ip"].asString();
        m_port = root["port"].asUInt();
        m_user = root["userName"].asString();
        m_passwd = root["password"].asString();
        m_dbName = root["dbName"].asString();
        m_minSize = root["minSize"].asInt();
        m_maxSize = root["maxSize"].asInt();
        m_maxIdleTime = root["maxIdleTime"].asInt();
        m_timeout = root["timeout"].asInt();
        return true;
    }
    return false;
}
