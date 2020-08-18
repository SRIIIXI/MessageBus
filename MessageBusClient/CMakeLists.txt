cmake_minimum_required(VERSION 3.5)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(Project MessageBusClient)
project(${Project})

set(MessageBus_VERSION_MAJOR 1)
set(MessageBus_VERSION_MINOR 0)

include_directories(./include ../CoreLib/include ../MessageBus/include /include/ /usr/include/ /usr/local/include/)

link_libraries(rt pthread dl)

set(SOURCE
${SOURCE}
../CoreLib/src/Base64.c
../CoreLib/src/StringEx.c
../CoreLib/src/StringList.c
../CoreLib/src/Responder.c
)

set(HEADERS
${HEADERS}

../CoreLib/include/Base64.h
../CoreLib/include/StringEx.h
../CoreLib/include/StringList.h
../CoreLib/include/Responder.h
../MessageBus/include/Payload.h
)

add_library(${Project} SHARED ${SOURCE} ${HEADERS} ./src/MessageBusClient.c ./include/MessageBusClient.h)

add_library(${Project}++ SHARED ${SOURCE} ${HEADERS} ./src/MessageBusClient.cpp ./include/MessageBusClient.hpp)


 
