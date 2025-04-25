cmake_minimum_required(VERSION 3.16)

# Project name and version
project(asio_coroutine_serve VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Boost libraries
find_package(Boost REQUIRED COMPONENTS system)
find_package(json-c REQUIRED)


# Add executable
add_executable(asio_coroutine_serve main.cpp 
    CService.cpp
    CSession.cpp
    LogicSystem.cpp
    MsgNode.cpp
    IOServicePool.cpp
    )

add_executable(asio_test_client test_client.cpp)

# Link Boost libraries
target_link_libraries(asio_coroutine_serve PRIVATE Boost::system jsoncpp)
target_link_libraries(asio_test_client PRIVATE Boost::system jsoncpp)


# Include directories
target_include_directories(asio_coroutine_serve PRIVATE ${Boost_INCLUDE_DIRS} ${jsoncpp_INCLUDE_DIRS})
target_include_directories(asio_test_client PRIVATE ${Boost_INCLUDE_DIRS} ${jsoncpp_INCLUDE_DIRS})

# # Set output directories
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
