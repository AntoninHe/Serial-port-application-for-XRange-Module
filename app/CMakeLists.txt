project(appDemo 
        DESCRIPTION "Very nice project"
        LANGUAGES CXX)

find_package (Threads REQUIRED)

add_executable(demoGateway 
    main.cpp
)

target_link_libraries(demoGateway PRIVATE seriallora ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(demoGateway PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)
