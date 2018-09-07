cmake_minimum_required(VERSION 3.11)
project (Serial_Lora)

set(CMAKE_CXX_STANDARD 11)

option(CLANG_TIDY_ON "Perform Clang-Tidy" ON)
find_program(
    CLANG_TIDY_EXE
    NAMES "clang-tidy"
    DOC "Path to clang-tidy executable"
)

if(CLANG_TIDY_EXE)
    if(CLANG_TIDY_ON)
            set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    endif()
endif()

add_subdirectory(src)
add_subdirectory(app)
