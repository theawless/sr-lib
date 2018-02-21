file(GLOB SOURCES src/*.cpp)
file(GLOB INCLUDES inc/*.h)

add_library(base ${SOURCES} ${INCLUDES})
target_include_directories(base PUBLIC "inc")
