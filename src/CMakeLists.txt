cmake_minimum_required(VERSION 3.11)
project(libseriallora 
        VERSION 1.0.0 
        DESCRIPTION "Very nice project"
        LANGUAGES CXX)

# Create target and set properties

add_library(seriallora
    serial_lora.cpp   
    forwarder.cpp     
    cpu.cpp
)

target_include_directories(seriallora PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/third_party/cppcodec)
