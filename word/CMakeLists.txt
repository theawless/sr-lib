include_directories(${SR_LIB_SOURCE_DIR}/base/inc)

file(GLOB SOURCES src/*.cpp)
file(GLOB INCLUDES inc/*.h)

add_library(word ${SOURCES} ${INCLUDES})
target_include_directories(word PUBLIC "inc")

target_link_libraries(word base)