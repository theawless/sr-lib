include_directories(${SR_LIB_SOURCE_DIR}/base/inc ${SR_LIB_SOURCE_DIR}/word/inc)

add_executable(WordPrediction src/word-prediction.cpp)
add_executable(WordRecognition src/word-recognition.cpp)

target_link_libraries(WordPrediction base word)
target_link_libraries(WordRecognition base word)

file(COPY res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
