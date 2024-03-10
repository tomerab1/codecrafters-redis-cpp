cmake_minimum_required(VERSION 3.13)
project(redis-starter-cpp)
set(CMAKE_CXX_STANDARD 20) # Enable the C++20 standard

set(SOURCE_FILES src/Server.cpp src/KeyValueStore.cpp
  src/Parser.cpp
  src/ResponseBuilder.cpp
  src/RedisServer.cpp 
  src/ProgramOptions.cpp 
  src/ProgramOptions.hpp
  src/Commands/EchoCommand.cpp
  src/Commands/GetCommand.cpp
  src/Commands/SetCommand.cpp
  src/Commands/PingCommand.cpp
  src/Commands/InfoCommand.cpp
  src/Commands/InvalidCommand.cpp
  src/Replication/ReplicationInfo.cpp
  )

add_executable(server ${SOURCE_FILES})
target_link_libraries(server pthread)