idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS ".")

option(BUILD_SHARED_LIBS OFF)
option(BUILD_TESTING OFF)
add_subdirectory(lib/data_validation_framework)
target_link_libraries(${COMPONENT_LIB} PUBLIC dvfw)