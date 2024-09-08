#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <MysqlDispatcher.h>

/// @brief 数据库连接池类（单例模式）
class ConnectionPool
{
public:
    /// @brief 返回数据库连接池的全局唯一对象指针
    /// @return 
    static ConnectionPool *getConnectPool();
    ConnectionPool(const ConnectionPool &con) = delete;
    ConnectionPool &operator=(const ConnectionPool &con) = delete;

private:
    ConnectionPool();
    /// @brief 解析json格式文件
    bool parseJsonFile();

private:
    std::string m_ip;
    std::string m_user;
    std::string m_passwd;
    std::string m_dbName;
    unsigned short m_port;
    int m_minSize;     // 数据库连接对象下限，默认创建的最小连接数
    int m_maxSize;     // 数据库连接对象上限
    int m_timeout;     // 超时时长，用于无可用连接时服务器的等待时长
    int m_maxIdleTime; // 数据库连接空闲时长
    std::queue<MysqlDispatcher *> m_connectionQ;
    std::mutex m_mutexQ;            // 互斥锁
    std::condition_variable m_cond; // 条件变量
};