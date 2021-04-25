project(kafka-demo)

add_executable(kafka-demo main.cpp)
target_link_libraries(kafka-demo
        RdKafka::rdkafka
        cppkafka
        )
