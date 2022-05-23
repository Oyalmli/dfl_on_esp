idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS ".")

option(BUILD_SHARED_LIBS OFF)
option(BUILD_TESTING OFF)
add_subdirectory(lib/data_flow_library)
target_link_libraries(${COMPONENT_LIB} PUBLIC dfl)