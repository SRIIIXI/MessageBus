cmake_minimum_required(VERSION 3.5)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(Project MessageBus)
project(${Project})

set(MessageBus_VERSION_MAJOR 1)
set(MessageBus_VERSION_MINOR 0)

include_directories(./include ../../CoreLib/include/ /include/ /usr/include/ /usr/local/include/)
link_directories(./lib/ /lib /usr/lib/ /usr/local/lib/ /usr/lib/${CMAKE_SYSTEM_PROCESSOR}/)

link_libraries(rt pthread dl)

set(SOURCE
${SOURCE}
../../CoreLib/src/Base64.c
../../CoreLib/src/Logger.c
../../CoreLib/src/SignalHandler.c
../../CoreLib/src/File.c
../../CoreLib/src/Directory.c
../../CoreLib/src/StringEx.c
../../CoreLib/src/StringList.c
../../CoreLib/src/Responder.c
../../CoreLib/src/ProcessLock.c
)

set(HEADERS
${HEADERS}
../../CoreLib/include/Base64.h
../../CoreLib/include/Logger.h
../../CoreLib/include/SignalHandler.h
../../CoreLib/include/StringEx.h
../../CoreLib/include/StringList.h
../../CoreLib/include/File.h
../../CoreLib/include/Directory.h
../../CoreLib/include/Responder.h
../../CoreLib/include/ProcessLock.h
)

add_executable(messagebusd ${SOURCE} ${HEADERS} ./src/MessageBroker.c ./src/main.c ./include/MessageBroker.h ./include/MessageBus.h)
add_library(messagebus SHARED ${SOURCE} ${HEADERS} ./src/MessageBus.c ./include/MessageBus.h)

install(TARGETS messagebusd DESTINATION local/bin)
install(TARGETS messagebus DESTINATION local/lib)
install(FILES ./include/MessageBus.h DESTINATION local/include/messagebus)

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Subrato Roy")

include(CPack)
