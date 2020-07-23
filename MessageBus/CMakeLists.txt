cmake_minimum_required(VERSION 3.5)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(Project MessageBus)
project(${Project})

set(MessageBus_VERSION_MAJOR 1)
set(MessageBus_VERSION_MINOR 0)

include_directories(./include/ ../CoreLib/include /include/ /usr/include/ /usr/local/include/)
link_directories(./lib/ /lib /usr/lib/ /usr/local/lib/ /usr/lib/${CMAKE_SYSTEM_PROCESSOR}/)

link_libraries(rt pthread dl)

set(SOURCE
${SOURCE}
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

