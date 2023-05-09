project(drop)

find_package(cdt)

add_contract( drop drop drop.cpp )
target_include_directories( drop PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( drop ${CMAKE_SOURCE_DIR}/../ricardian )