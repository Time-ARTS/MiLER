cmake_minimum_required(VERSION 3.16)
project(MiLER VERSION 1.1.0 LANGUAGES CXX)

# ---- Compiler Settings ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Find OpenCV ----
# Adjust OpenCV_DIR path as necessary
# This is primarily for use other operators than the RPI5
if(WIN32)
    set(OpenCV_DIR "C:/opencv/build/x64/vc16/lib")  # <-- adjust your path
endif()

find_package(OpenCV REQUIRED)

# ---- Find WiringPi (Linux only) ----
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
    motorControll/PID.cpp 
    motorControll/vectorTranslator.cpp
    motorControll/motorController.cpp
    discordMessaging/discordMessaging.cpp
)

# ---- Header Files ----
set(HEADERS
    constructs.hpp 
    loop.hpp
    settings.hpp 
    vision/vision.hpp 
    vision/quadrantHandler.hpp 
    motorControll/PID.hpp 
    motorControll/vectorTranslator.hpp
    motorControll/motorController.hpp
    discordMessaging/discordMessaging.hpp
)

# ---- Create Executable ----
add_executable(MiLER ${SOURCES} ${HEADERS})

# ---- Link Libraries ----
target_link_libraries(MiLER
    ${OpenCV_LIBS}
)

# ---- Link WiringPi if found (Linux only) ----
if(WIRINGPI_LIB)
    target_link_libraries(MiLER ${WIRINGPI_LIB})
endif()