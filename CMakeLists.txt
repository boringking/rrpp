cmake_minimum_required(VERSION 3.6)
project(RRPP)


set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS "-std=c99")
set(CMAKE_EXE_LINKER_FLAGS -pthread)

# cross-compiler
set(CROSS /opt/acrospeed/mips-linux-uclibc/bin/mips-linux-)
set(CMAKE_AR         ${CROSS}ar)
set(CMAKE_C_COMPILER ${CROSS}cc)
set(CMAKE_LINKER     ${CROSS}ld)
set(CMAKE_NM         ${CROSS}nm)
set(CMAKE_OBJCOPY    ${CROSS}objcopy)
set(CMAKE_OBJDUMP    ${CROSS}objdump)
set(CMAKE_RANLIB     ${CROSS}ranlib)
set(CMAKE_STRIP      ${CROSS}strip)


include_directories(${PROJECT_SOURCE_DIR}/kit)
include_directories(${PROJECT_SOURCE_DIR}/node)
include_directories(${PROJECT_SOURCE_DIR}/protocol)
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/icplib/include)
include_directories(${PROJECT_SOURCE_DIR}/icplib/lib/libportseq)
include_directories(${PROJECT_SOURCE_DIR}/icplib/driver/switchdriver)
link_directories(${PROJECT_BINARY_DIR}/libs)
link_directories(${PROJECT_SOURCE_DIR}/icplib)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/libs)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/elf)

add_subdirectory(kit)
add_subdirectory(protocol)
add_subdirectory(node)
add_subdirectory(src)

