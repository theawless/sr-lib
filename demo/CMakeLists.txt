include_directories(${SR_LIB_SOURCE_DIR}/sr-lib/inc)

add_executable(Recorder src/recorder.cpp)
add_executable(WordRecognizer src/word-recognizer.cpp)
add_executable(WordRecognizerParallel src/word-recognizer-parallel.cpp)

target_link_libraries(Recorder sr-lib)
target_link_libraries(WordRecognizer sr-lib)
target_link_libraries(WordRecognizerParallel sr-lib)
