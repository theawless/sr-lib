include_directories(${SR_LIB_SOURCE_DIR}/sr-lib/inc)

add_executable (DigitHmm src/digit-hmm.cpp)
add_executable (DigitPhoneme src/digit-phoneme.cpp)
add_executable (Vowel src/vowel.cpp)
add_executable (Boolean src/boolean.cpp)
add_executable (Recorder src/recorder.cpp)

target_link_libraries(DigitHmm sr-lib)
target_link_libraries(DigitPhoneme sr-lib)
target_link_libraries(Vowel sr-lib)
target_link_libraries(Boolean sr-lib)
target_link_libraries(Recorder sr-lib)
