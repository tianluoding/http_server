#include "sql_connection_pool.h"

connection_pool::connection_pool()
{
    this->m_CurConn = 0;
    this->m_FreeConn = 0;
}

connection_pool::~connection_pool()
{
    DestroyPool();
}
connection_pool *connection_pool::GetInstance()
{
    static connection_pool connPool;
    return &connPool;
}

void connection_pool::init(string url, string User, string PassWord, string DataBaseName, int Port, int MaxConn, int close_log)
{
    this->m_url = url;
    this->m_Port = Port;
    this->m_User = User;
    this->m_PassWord = PassWord;
    this->m_DatabaseName = DataBaseName;
    this->m_close_log = close_log;

    for(int i = 0; i < MaxConn; i++)
    {
        MYSQL *con = NULL;
        con = mysql_init(con);

        if(con == NULL)
        {
            cout << "Error;" << mysql_error(con);
            exit(1);
        }
        con = mysql_real_connect(con, m_url.c_str(), m_User.c_str(), m_PassWord.c_str(), m_DatabaseName.c_str(), m_Port, NULL, 0);

        if(con == NULL)
        {
            cout << "Error:" << mysql_error(con);
            exit(1);
        }

        connList.push_back(con);
        ++m_FreeConn;
    }

    reserve = sem(m_FreeConn);
    this->m_MaxConn = m_FreeConn;
}

MYSQL *connection_pool::GetConnection()
{
    MYSQL *con = NULL;

    if(0 == connList.size())
        return NULL;

    reserve.wait();

    lock.lock();

    con = connList.front();
    connList.pop_front();

    --m_FreeConn;
    ++m_CurConn;

    lock.unlock();
    return con;
}

bool connection_pool::ReleaseConnection(MYSQL *con)
{
    if(con == NULL)
        return false;

    lock.lock();

    connList.push_back(con);

    ++m_FreeConn;
    --m_CurConn;

    lock.unlock();

    reserve.post();
    return true;
}

void connection_pool::DestroyPool()
{
    lock.lock();
    if(connList.size() > 0)
    {
        list<MYSQL *>::iterator it;
        for(it = connList.begin(); it != connList.end(); it++)
        {
            MYSQL *con = *it;
            mysql_close(con);
        }
        m_CurConn = 0;
        m_FreeConn = 0;

        connList.clear();

        lock.unlock();
    }
    lock.unlock();
}

connectionRAII::connectionRAII(MYSQL **con, connection_pool *connPool)
{
    *con = connPool->GetConnection();

    conRAII = *con;
    poolRAII = connPool;
}

connectionRAII::~connectionRAII()
{
    poolRAII->ReleaseConnection(conRAII);
}