
# Now simply link against gtest or gtest_main as needed. Eg
target_sources(Vajolet_unit_test PRIVATE LR-test.cpp sparseEntry-test.cpp tbfile-test.cpp tbtable-test.cpp tbvalidater-test.cpp)


target_sources(Vajolet_syzygy_test PRIVATE syzygy-functional-test.cpp)


