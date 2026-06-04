cmake_minimum_required(VERSION 3.16)
project(MiLER VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)

# ---- Compiler Settings ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "Configuring for platform: ${CMAKE_SYSTEM_NAME}")

# ---- OpenCV ----
if (WIN32)
    set(OpenCV_DIR "C:/opencv/build/x64/vc16/lib") # Change to your OpenCV install
endif ()
find_package(OpenCV REQUIRED)

# ---- WiringPi (RPi only) ----
if (UNIX AND NOT APPLE)
    find_library(WIRINGPI_LIB NAMES wiringPi)
endif ()

add_executable(MiLER
        loop.cpp
        run.cpp
        config.cpp
        shortKeyCommands/shortKeyCommands.cpp
        vision/vision.cpp
        vision/debugRenderer.cpp
        vision/quadrantHandler.cpp
        vision/perspectiveTuner.cpp
        motorControl/PID.cpp
        motorControl/controlExtractor.cpp
        motorControl/motorController.cpp
)

# ---- Include Directories ----
target_include_directories(MiLER PRIVATE
        ${OpenCV_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}
)

# ---- Windows: ensure OpenCV DLLs are found at runtime ----
if (WIN32)
    add_custom_command(TARGET MiLER POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${OpenCV_DIR}/../bin"
            $<TARGET_FILE_DIR:MiLER>
    )
endif ()

# ---- Link Libraries ----
target_link_libraries(MiLER ${OpenCV_LIBS})

# ---- WiringPi ----
if (WIRINGPI_LIB)
    target_link_libraries(MiLER ${WIRINGPI_LIB})
endif ()