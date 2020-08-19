cmake_minimum_required(VERSION 3.5)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(Project MessageBus)
project(${Project})

set(MessageBus_VERSION_MAJOR 1)
set(MessageBus_VERSION_MINOR 0)

include_directories(./include/ /include/ /usr/include/ /usr/local/include/)
link_directories(./lib/ /lib /usr/lib/ /usr/local/lib/ /usr/lib/${CMAKE_SYSTEM_PROCESSOR}/)

link_libraries(rt pthread dl)

set(SOURCE
${SOURCE}
./src/Base64.c
./src/Logger.c
./src/SignalHandler.c
./src/File.c
./src/Directory.c
./src/StringEx.c
./src/StringList.c
./src/Responder.c
)

set(HEADERS
${HEADERS}
./include/Base64.h
./include/Logger.h
./include/SignalHandler.h
./include/StringEx.h
./include/StringList.h
./include/File.h
./include/Directory.h
./include/Responder.h
)

add_executable(messagebusd ${SOURCE} ${HEADERS} ./src/MessageBroker.c ./src/main.c ./include/MessageBroker.h ./include/MessageBus.h)
add_library(messagebus SHARED ${SOURCE} ${HEADERS} ./src/MessageBus.c ./include/MessageBus.h)

install(TARGETS messagebusd DESTINATION local/bin)
install(TARGETS messagebus DESTINATION local/lib)
install(FILES ./include/MessageBus.h DESTINATION local/include/messagebus)

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Subrato Roy")

include(CPack)
