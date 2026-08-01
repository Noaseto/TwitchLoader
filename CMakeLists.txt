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

# Unsure of how to declare those
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(twitch_loader PRIVATE Boost::system)

find_package(Threads REQUIRED)
target_link_libraries(twitch_loader PRIVATE Threads::Threads)

find_package(OpenSSL REQUIRED)
target_link_libraries(twitch_loader PRIVATE OpenSSL::SSL OpenSSL::Crypto)