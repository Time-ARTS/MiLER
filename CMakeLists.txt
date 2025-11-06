cmake_minimum_required(VERSION 3.16)  # 3.16+ is widely supported on RPi OS
project(MiLER VERSION 1.0.0 LANGUAGES CXX)

# ---- Compiler Settings ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Find Packages ----
find_package(OpenCV REQUIRED)

find_library(WIRINGPI_LIB wiringPi)
find_library(THREAD_LIB pthread REQUIRED)  # Threading support

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
    motorControll/MotorController.cpp
)

# ---- Header Files (for IDEs, optional) ----
set(HEADERS
    constructs.hpp 
    loop.hpp
    settings.hpp 
    vision/vision.hpp 
    vision/quadrantHandler.hpp 
    motorControll/PID.hpp 
    motorControll/vectorTranslator.hpp
    motorControll/MotorController.hpp
)

# ---- Create Executable ----
add_executable(MiLER ${SOURCES} ${HEADERS})

# ---- Link Libraries ----
target_link_libraries(MiLER
    ${OpenCV_LIBS}
    ${THREAD_LIB}
    ${WIRINGPI_LIB}
)

# ---- Optional Debug Output ----
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
