idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES "data_inp.txt")

cmake_minimum_required(VERSION 3.5)

option(BUILD_SHARED_LIBS OFF)
option(BUILD_TESTING OFF)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib/data_flow_library)
target_link_libraries(${COMPONENT_LIB} PUBLIC dfl)