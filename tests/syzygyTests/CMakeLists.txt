
# Now simply link against gtest or gtest_main as needed. Eg
target_sources(Vajolet_unit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/LR-test.cpp ${CMAKE_CURRENT_SOURCE_DIR}/sparseEntry-test.cpp ${CMAKE_CURRENT_SOURCE_DIR}/tbfile-test.cpp ${CMAKE_CURRENT_SOURCE_DIR}/tbtable-test.cpp ${CMAKE_CURRENT_SOURCE_DIR}/tbvalidater-test.cpp)


target_sources(Vajolet_syzygy_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/syzygy-functional-test.cpp)


