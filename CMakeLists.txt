cmake_minimum_required(VERSION 3.16)
project(MiLER VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)

# ---- Compiler Settings ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "Configuring for platform: ${CMAKE_SYSTEM_NAME}")

# ---- OpenCV ----
if(WIN32)
    set(OpenCV_DIR "C:/opencv/build/x64/vc16/lib") # Change to your OpenCV install
endif()
find_package(OpenCV REQUIRED)

# ---- CURL ----
if(UNIX AND NOT APPLE)
    find_package(CURL REQUIRED)
    set(CURL_TARGET CURL::libcurl)
    message(STATUS "Using system libcurl")
else()
    message(STATUS "Fetching curl via FetchContent")
    FetchContent_Declare(
            curl
            GIT_REPOSITORY https://github.com/curl/curl.git
            GIT_TAG curl-8_7_1
    )
    # Ensure we build only the library, not the executable
    set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
    set(CURL_STATICLIB ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(curl)

    set(CURL_TARGET libcurl)
endif()

# ---- WiringPi (RPi only) ----
if(UNIX AND NOT APPLE)
    find_library(WIRINGPI_LIB NAMES wiringPi)
endif()

# ---- Include Directories ----
include_directories(
        ${OpenCV_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}
)

# ---- Source Files ----
set(SOURCES
        loop.cpp
        run.cpp
        vision/vision.cpp
        vision/quadrantHandler.cpp
        motorControl/PID.cpp
        motorControl/vectorTranslator.cpp
        motorControl/motorController.cpp
        discordMessaging/discordMessaging.cpp
)

set(HEADERS
        constructs.hpp
        loop.hpp
        settings.hpp
        vision/vision.hpp
        vision/quadrantHandler.hpp
        motorControl/PID.hpp
        motorControl/vectorTranslator.hpp
        motorControl/motorController.hpp
        discordMessaging/discordMessaging.hpp
)

# ---- Executable ----
add_executable(MiLER ${SOURCES} ${HEADERS})

# ---- Windows: ensure OpenCV DLLs are found at runtime ----
if(WIN32)
    add_custom_command(TARGET MiLER POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${OpenCV_DIR}/../bin"
            $<TARGET_FILE_DIR:MiLER>
    )
endif()

# ---- Link Libraries ----
target_link_libraries(MiLER
        ${OpenCV_LIBS}
        ${CURL_TARGET}
)

# ---- WiringPi ----
if(WIRINGPI_LIB)
    target_link_libraries(MiLER ${WIRINGPI_LIB})
endif()