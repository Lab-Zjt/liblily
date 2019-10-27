cmake_minimum_required(VERSION 3.10)
project(lily)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS " -CC -Wall -Wextra -Werror -Wno-unused-parameter -Wno-sign-compare -Wno-unused-result -Wno-unused-value -Wno-unused-variable -pedantic -g -rdynamic -fPIC")
#set(CMAKE_BUILD_TYPE RELEASE)

include_directories(/usr/local/include/)
link_directories(/usr/local/lib/boost/lib/)


file(GLOB hook_src src/hook/*.cpp)
file(GLOB net_src src/net/*.cpp src/net/http/*.cpp)
file(GLOB lily_src src/coroutine/*.cpp)

add_library(hook ${hook_src})
add_library(net ${net_src})
add_library(lily ${lily_src})

add_executable(server server.cpp)
add_executable(client client.cpp)
add_executable(http_server src/app/http_server.cpp)

target_link_libraries(lily hook net pthread boost_context dl rt)
target_link_libraries(server lily)
target_link_libraries(client lily)
target_link_libraries(http_server lily)