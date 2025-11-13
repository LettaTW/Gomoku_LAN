cmake_minimum_required(VERSION 3.10)
project(GomokuServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 讓 CMake 找到 nlohmann/json 函式庫
include_directories(lib)

# 找到執行緒函式庫 (用於 std::thread)
find_package(Threads REQUIRED)

# 加入可執行檔
add_executable(
    GomokuServer
    src/main.cpp
    src/NetworkServer.cpp
    src/Protocol.cpp
    src/GameLogic.cpp
)

# 連結函式庫
target_link_libraries(GomokuServer PRIVATE Threads::Threads)