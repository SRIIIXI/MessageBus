cmake_minimum_required(VERSION 3.5)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(Project Pumba)
project(${Project})

include_directories(./include/ ../../CoreLib/include/ ../../MessageBus/include/ ../../MessageBusClient/include/ /include/ /usr/include/ /usr/local/include/)

if(WIN32)
link_libraries(ws2_32)
else()
link_libraries(rt pthread dl)
endif()

set(SOURCE
${SOURCE}
../../CoreLib/src/Base64.c
../../CoreLib/src/StringEx.c
../../CoreLib/src/StringList.c
../../CoreLib/src/Responder.c
../../MessageBusClient/src/MessageBusClient.c
./src/main.c
)

set(HEADERS
${HEADERS}
../../CoreLib/include/Base64.h
../../CoreLib/include/StringEx.h
../../CoreLib/include/StringList.h
../../CoreLib/include/Responder.h
../../MessageBus/include/Payload.h
../../MessageBusClient/include/MessageBusClient.h
)

add_executable(${Project}  ${SOURCE} ${HEADERS})
