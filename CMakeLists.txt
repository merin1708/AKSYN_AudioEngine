cmake_minimum_required(VERSION 3.10)
project(AKSYN_AudioEngine)

set(CMAKE_CXX_STANDARD 17)

set(BOOST_ROOT "C:/local/boost_1_84_0")

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# Build Sender
add_executable(node_a_sender node_a_sender.cpp)
target_link_libraries(node_a_sender ${Boost_LIBRARIES})

# Build Receiver
add_executable(node_b_receiver node_b_receiver.cpp)
target_link_libraries(node_b_receiver ${Boost_LIBRARIES})