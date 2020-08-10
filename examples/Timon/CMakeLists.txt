cmake_minimum_required(VERSION 3.5)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(Project Timon)
project(${Project})

include_directories(./include/ ../../CoreLib/include/ ../../MessageBus/include/ ../../MessageBusClient/include/ /include/ /usr/include/ /usr/local/include/)

if(WIN32)
link_libraries(ws2_32)
if (MSVC)
    # Build cpp files on all cores
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /W4")

    # can't use the "secure" versions as they're Windows specific
    add_definitions("/D\"_CRT_SECURE_NO_WARNINGS\"")
    add_definitions("/D\"_SCL_SECURE_NO_WARNINGS\"")
endif()
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
