cmake_minimum_required(VERSION 3.5)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(Project MessageBus)
project(${Project})

set(MessageBus_VERSION_MAJOR 1)
set(MessageBus_VERSION_MINOR 0)

include_directories(./include/ ../CoreLib/include /include/ /usr/include/ /usr/local/include/)
link_directories(./lib/ /lib /usr/lib/ /usr/local/lib/ /usr/lib/${CMAKE_SYSTEM_PROCESSOR}/)

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
../CoreLib/src/Base64.c
../CoreLib/src/Logger.c
../CoreLib/src/SignalHandler.c
../CoreLib/src/File.c
../CoreLib/src/Directory.c
../CoreLib/src/StringEx.c
../CoreLib/src/StringList.c
../CoreLib/src/Responder.c
./src/MessageBroker.c
./src/main.c
)

set(HEADERS
${HEADERS}
../CoreLib/include/OSDefinitions.h
../CoreLib/include/Base64.h
../CoreLib/include/Logger.h
../CoreLib/include/SignalHandler.h
../CoreLib/include/StringEx.h
../CoreLib/include/StringList.h
../CoreLib/include/File.h
../CoreLib/include/Directory.h
./include/Payload.h
./include/MessageBroker.h
./include/Responder.h
)

add_executable(${Project} ${SOURCE})

