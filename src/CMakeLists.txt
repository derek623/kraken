set(CMAKE_VERBOSE_MAKEFILE ON)
#set(BINARY ${CMAKE_PROJECT_NAME})

#file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(SOURCES main.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
target_link_libraries(${CMAKE_PROJECT_NAME} ${Boost_SYSTEM_LIBRARY} Threads::Threads)

add_executable(client client.cpp)
target_link_libraries(client ${Boost_SYSTEM_LIBRARY}  Threads::Threads)