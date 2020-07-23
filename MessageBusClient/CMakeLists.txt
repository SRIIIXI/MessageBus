cmake_minimum_required(VERSION 3.5)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(Project MessageBusClient)
project(${Project})

set(MessageBus_VERSION_MAJOR 1)
set(MessageBus_VERSION_MINOR 0)

include_directories(./include ../CoreLib/include ../MessageBus/include /include/ /usr/include/ /usr/local/include/)

set(SOURCE
${SOURCE}
../CoreLib/src/Base64.c
../CoreLib/src/StringEx.c
../CoreLib/src/StringList.c
../CoreLib/src/Responder.c
./src/MessageBusClient.c
)

set(HEADERS
${HEADERS}
../CoreLib/include/Base64.h
../CoreLib/include/StringEx.h
../CoreLib/include/StringList.h
../CoreLib/include/Responder.h
../MessageBus/include/Payload.h
./include/MessageBusClient.h
)

add_library(${Project} SHARED ${SOURCE} ${HEADERS})

 
