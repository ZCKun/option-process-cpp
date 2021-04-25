//
// Created by x2h1z on 4/16/21.
//

#ifndef OPTION_PROCESS_SQL_H
#define OPTION_PROCESS_SQL_H

#include <mutex>
#include "mysql-cppconn-8/mysqlx/xdevapi.h"

class Sql
{
public:
    ~Sql()
    {
        session_.commit();
        session_.close();
    }

    Sql(const Sql &) = delete;

    Sql &operator=(const Sql &) = delete;

    static Sql &instance()
    {
        static Sql ins;
        return ins;
    }

    mysqlx::Session &get_session()
    {
        return session_;
    }

    void settings(const std::string& settings)
    {
    }

    /**
     * 获取数据库, 不存在将自动创建
     * @param schema_name 数据库名
     * @param created 不存在是否创建
     * @return
     */
    mysqlx::Schema get_schema(const std::string &schema_name);

    mysqlx::SqlResult execute_sql_command(const std::string &query);


private:

    mysqlx::Session session_;
    std::mutex mutex_;

    Sql() : session_("mysqlx://xzhong:xzhong@192.168.0.66:33060/?ssl-mode=required")
    {}
};


#endif //OPTION_PROCESS_SQL_H
