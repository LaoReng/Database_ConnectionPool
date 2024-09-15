#include "ConnectionPool.h"
#include "json.h"
#include <fstream>
#include <thread>

std::shared_ptr<MysqlDispatcher> ConnectionPool::getConnection()
{
    std::unique_lock<std::mutex> locker(m_mutexQ);
    while (m_connectionQ.empty())
    {
        if (std::cv_status::timeout == m_cond.wait_for(locker, std::chrono::milliseconds(m_timeout)))
        {
            // 等待时间超时，说明任务队列依旧为空
            if (m_connectionQ.empty())
            {
                // return nullptr;
                continue; // 一直阻塞的做法
            }
        }
    }

    std::shared_ptr<MysqlDispatcher> connPtr(m_connectionQ.front(),
                                             [this](MysqlDispatcher *conn)
                                             {
                                                 {
                                                     std::lock_guard<std::mutex> locker(m_mutexQ); // 和unique_lock的用法类似
                                                     // std::unique_lock<std::mutex> locker(m_mutexQ);
                                                     conn->refreshAliveTime();
                                                     m_connectionQ.push(conn);
                                                 }
                                             }); // 共享指针可以指定删除器（对象销毁）的动作
    m_connectionQ.pop();
    m_cond.notify_all(); // 唤醒生产者
    return connPtr;
}

ConnectionPool::~ConnectionPool()
{
    while (m_connectionQ.empty())
    {
        MysqlDispatcher *conn = m_connectionQ.front();
        m_connectionQ.pop();
        delete conn;
    }
}

ConnectionPool::ConnectionPool()
{
    // 加载配置文件
    if (!parseJsonFile())
    {
        return;
    }
    for (int i = 0; i < m_minSize; ++i)
    {
        addConnection();
    }
    // 生产连接池里面的连接
    std::thread producer(&ConnectionPool::produceConnection, this);
    // 检测有没有需要销毁的连接
    std::thread recycler(&ConnectionPool::recycleConnection, this);

    // 子线程与主线程分离，不阻塞主线程
    producer.detach();
    recycler.detach();
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

void ConnectionPool::produceConnection()
{
    while (true)
    {
        std::unique_lock<std::mutex> locker(m_mutexQ);
        while (m_connectionQ.size() >= m_minSize)
        {
            m_cond.wait(locker);
        }
        addConnection();
        m_cond.notify_all(); // 唤醒消费者
    }
}

void ConnectionPool::recycleConnection()
{
    while (true)
    {
        {
            std::lock_guard<std::mutex> locker(m_mutexQ);
            while (m_connectionQ.size() > m_minSize)
            {
                // 取出对头的元素
                MysqlDispatcher *conn = m_connectionQ.front();
                if (conn->getAliveTime() >= m_maxIdleTime)
                {
                    m_connectionQ.pop();
                    delete conn;
                }
                else
                {
                    break;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void ConnectionPool::addConnection()
{
    // TODO: 这里的处理有问题，有没有可能这个连接创建失败了
    MysqlDispatcher *conn = new MysqlDispatcher;
    if (conn->connect(m_user, m_passwd, m_dbName, m_ip, m_port))
        m_connectionQ.push(conn);
    else
    {
        // TODO: 可以输出错误信息之类的
        delete conn;
    }
}
