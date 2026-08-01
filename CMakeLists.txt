cmake_minimum_required(VERSION 3.26)
project(twitch_loader CXX)

# The Dusklight version to build against: a release tag (e.g. "v1.5.0") or a commit SHA.
if (NOT DUSKLIGHT_VERSION)
    set(DUSKLIGHT_VERSION "4504e5009f22a8d32b2dc4b7470a543a9e2bafe4")
endif ()
# Fetches the Dusklight sources into ./dusklight and provides DUSKLIGHT_DIR.
# Use -DDUSKLIGHT_DIR= to point at an existing Dusklight checkout.
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchDusklight.cmake")
add_subdirectory("${DUSKLIGHT_DIR}/sdk" dusklight-sdk EXCLUDE_FROM_ALL)

add_mod(twitch_loader
        SOURCES src/mod.cpp
        MOD_JSON mod.json
        RES_DIR res            # mod resources, including icon.png and banner.png
)

include(FetchContent)
set(BUILD_SHARED_LIBS OFF)
set(BOOST_INCLUDE_LIBRARIES beast asio system)
set(BOOST_ENABLE_CMAKE ON)
FetchContent_Declare(
        Boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-cmake.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(Boost)
target_link_libraries(twitch_loader PRIVATE Boost::beast Boost::asio Boost::system)

find_package(Threads REQUIRED)
target_link_libraries(twitch_loader PRIVATE Threads::Threads)

find_package(OpenSSL REQUIRED)
target_link_libraries(twitch_loader PRIVATE OpenSSL::SSL OpenSSL::Crypto)