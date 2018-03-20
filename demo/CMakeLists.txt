include_directories(${SR_LIB_SOURCE_DIR}/word/inc)

add_executable(OptimiseWordRecognition src/optimise-word-recognition.cpp)
add_executable(LiveWordRecognition src/live-word-recognition.cpp)
add_executable(TestWordRecognition src/test-word-recognition.cpp)

target_link_libraries(OptimiseWordRecognition word)
target_link_libraries(LiveWordRecognition word)
target_link_libraries(TestWordRecognition word)
