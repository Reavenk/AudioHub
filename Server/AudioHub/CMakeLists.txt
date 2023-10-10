cmake_minimum_required(VERSION 3.12)
project(AudioHubServer)

# Specify that we are building for Windows x64
set(CMAKE_GENERATOR_PLATFORM x64)

# Find source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Set the C++ standard
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()

# Require Boost 1.8
find_package(Boost 1.8 REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif()

# Require OpenSSL 1.1
find_package(OpenSSL "1.1" REQUIRED)
if(OpenSSL_FOUND)
	message(STATUS "OPENSSL_ROOT_DIR: ${OPENSSL_ROOT_DIR}")
	message(STATUS "OPENSSL_INCLUDE_DIR: ${OPENSSL_INCLUDE_DIR}")
	message(STATUS "OPENSSL_LIBRARIES: ${OPENSSL_LIBRARIES}")
    include_directories(${OpenSSL_INCLUDE_DIR})
	target_link_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_ROOT_DIR}/lib)
    target_link_libraries(${PROJECT_NAME} ${OpenSSL_LIBRARIES})
	target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIR})
endif()


# Link against libcrypto.lib and libssl.lib
# (This may be redundant because OpenSSL_LIBRARIES might include them)
target_link_libraries(${PROJECT_NAME} libcrypto.lib libssl.lib)

# Add the necessary preprocessor definitions for a Windows Console application
target_compile_definitions(${PROJECT_NAME} PRIVATE _CONSOLE _WIN32_WINNT=0x0601)