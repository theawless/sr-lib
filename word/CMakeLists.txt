file(GLOB SOURCES src/*.cpp)
file(GLOB INCLUDES inc/*.h)

add_library(word ${SOURCES} ${INCLUDES})
target_include_directories(word PUBLIC inc)

target_link_libraries(word base)
target_include_directories(word PRIVATE ${SR_LIB_SOURCE_DIR}/base/inc)
