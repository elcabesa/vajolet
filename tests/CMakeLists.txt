enable_testing()
# Now simply link against gtest or gtest_main as needed. Eg
add_executable(Vajolet_unit_test 
	book-test.cpp
    commandTest.cpp
	dataTest.cpp
	hashKeyTest.cpp
	historyTest.cpp
	main.cpp
	MoveTest.cpp
	MoveListTest.cpp
	multiPVmanagerTest.cpp
	nnueTest.cpp
	perft-test.cpp
	pvLineFollowerTest.cpp
	positionTest.cpp
	pvLineTest.cpp
	searchTimer-test.cpp
	see-test.cpp
	timeManagement-test.cpp
	UciOutput-test.cpp)

target_link_libraries(Vajolet_unit_test gtest libChess)
target_include_directories (Vajolet_unit_test PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(Vajolet_syzygy_test main.cpp)
target_link_libraries(Vajolet_syzygy_test gtest libChess)
target_include_directories (Vajolet_syzygy_test PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(Vajolet_long_tests main.cpp perft-test-long.cpp)
target_link_libraries(Vajolet_long_tests gtest libChess)
target_include_directories (Vajolet_long_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(Vajolet_search_integration_test main.cpp search-integration-test.cpp )
target_link_libraries(Vajolet_search_integration_test gtest libChess)
target_include_directories (Vajolet_search_integration_test PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(Vajolet_perft_validation main.cpp perft-test-very-long.cpp )
target_link_libraries(Vajolet_perft_validation gtest libChess)
target_include_directories (Vajolet_perft_validation PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(Vajolet_wac wac.cpp )
target_link_libraries(Vajolet_wac libChess)
target_include_directories (Vajolet_wac PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_subdirectory(syzygyTests)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Perfect2017.bin
    ${CMAKE_CURRENT_BINARY_DIR}/book.bin COPYONLY)
	
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fischer.txt
    ${CMAKE_CURRENT_BINARY_DIR}/fischer.txt COPYONLY)
	
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/perft.txt
    ${CMAKE_CURRENT_BINARY_DIR}/perft.txt COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/wacnew.epd
    ${CMAKE_CURRENT_BINARY_DIR}/wacnew.epd COPYONLY)
    
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/STS1-STS15_LAN_v3.epd
    ${CMAKE_CURRENT_BINARY_DIR}/STS1-STS15_LAN_v3.epd COPYONLY)

add_test(NAME unit_test COMMAND Vajolet_unit_test)
add_test(NAME syzygy_test COMMAND Vajolet_syzygy_test)
add_test(NAME long_tests COMMAND Vajolet_long_tests)
add_test(NAME search_integration_tests COMMAND Vajolet_search_integration_test)



add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS Vajolet_unit_test Vajolet_syzygy_test Vajolet_long_tests Vajolet_search_integration_test)
