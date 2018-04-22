add_executable(WordPrediction src/word-prediction.cpp)
add_executable(WordRecognition src/word-recognition.cpp)

target_link_libraries(WordPrediction base word)
target_include_directories(WordPrediction PRIVATE ${SR_LIB_SOURCE_DIR}/word/inc)
target_link_libraries(WordRecognition base word)
target_include_directories(WordRecognition PRIVATE ${SR_LIB_SOURCE_DIR}/word/inc)

# copy resources folder so that the demo can use relative paths
file(COPY res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
