//
// Created by x2h1z on 4/22/21.
//
#include <iostream>
#include "mysql-cppconn-8/jdbc/cppconn/driver.h"
#include "mysql-cppconn-8/jdbc/cppconn/exception.h"
#include "mysql-cppconn-8/jdbc/cppconn/resultset.h"
#include "mysql-cppconn-8/jdbc/cppconn/statement.h"
#include "mysql-cppconn-8/jdbc/cppconn/prepared_statement.h"
#include "cppkafka/cppkafka.h"

int main2()
{
    cppkafka::Configuration config {
            {"metadata.broker.list", "192.168.0.66:9092"}
    };

    auto producer = std::make_shared<cppkafka::Producer>(config);

    std::string message("from cppkafka");
    producer->produce(
            cppkafka::MessageBuilder("option-demo")
            .partition(0).payload(message));
    std::cout << "hello" << std::endl;
    producer->flush();

    std::cout << "hello" << std::endl;

    return 0;
}

int main()
{
    sql::Driver *driver;
    sql::Connection *con;
    sql::Statement *stmt;
    sql::ResultSet *res;
    sql::PreparedStatement *pstmt;

    driver = get_driver_instance();
    driver->connect("tcp:://192.168.0.66:3307", "xzhong", "xzhong");

    return 0;
}