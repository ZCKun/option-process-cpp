//
// Created by x2h1z on 4/16/21.
//

#include "Sql.h"

mysqlx::SqlResult Sql::execute_sql_command(const std::string &query)
{
    std::lock_guard<std::mutex> guard(mutex_);
    auto result = session_.sql(query).execute();
    session_.commit();
    return result;
}

mysqlx::Schema Sql::get_schema(const std::string &schema_name)
{
    std::lock_guard<std::mutex> guard(mutex_);
    try {
        auto temp = session_.getSchema(schema_name, true);
        return temp;
    } catch (...) {
        // 不存在, 创建
        std::string statement("create scheme");
        statement += schema_name;
        statement += ";";

        auto result = session_.sql(statement).execute();
        auto temp = session_.getSchema(schema_name, true);
        return temp;
    }
}


