file(GLOB SOURCES src/*.cpp)
file(GLOB INCLUDES inc/*.h)

add_library(sr-lib ${SOURCES} ${INCLUDES})
target_include_directories(sr-lib PUBLIC "inc")
