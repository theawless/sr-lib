include_directories(${SR_LIB_SOURCE_DIR}/sr-lib/inc)

add_executable(Word src/word.cpp)
add_executable(Vowel src/vowel.cpp)
add_executable(Recorder src/recorder.cpp)

target_link_libraries(Word sr-lib)
target_link_libraries(Vowel sr-lib)
target_link_libraries(Recorder sr-lib)
