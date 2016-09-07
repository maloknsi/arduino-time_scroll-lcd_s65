set(SDK_PATH_HINTS "E:\\MyProg\\_Elektronics\\IDE\\AVR\\arduino-1.6.3")
set(ARDUINO_CPU 16MHzatmega328)
cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)

set(PROJECT_NAME s65_3)

project(${PROJECT_NAME})

set(${PROJECT_NAME}_BOARD pro)
set(${PROJECT_NAME}_PORT COM3)

set(${CMAKE_PROJECT_NAME}_SKETCH s65_3.ino)

link_directories(${ARDUINO_SDK}/libraries)
generate_arduino_firmware(${CMAKE_PROJECT_NAME})
