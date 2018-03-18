include_directories(${SR_LIB_SOURCE_DIR}/word/inc)

add_executable(OptimisingRecognition src/optimising-recognition.cpp)
add_executable(LiveWordRecognition src/live-word-recognition.cpp)
add_executable(TestWordRecognition src/test-word-recognition.cpp)

target_link_libraries(OptimisingRecognition word)
target_link_libraries(LiveWordRecognition word)
target_link_libraries(TestWordRecognition word)
