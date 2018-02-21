include_directories(${SR_LIB_SOURCE_DIR}/word/inc)

add_executable(LiveWordRecognition live-word-recognition.cpp)
add_executable(TestWordRecognition Test-word-recognition.cpp)

target_link_libraries(LiveWordRecognition word)
target_link_libraries(TestWordRecognition word)
