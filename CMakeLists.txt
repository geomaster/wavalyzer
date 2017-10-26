cmake_minimum_required(VERSION 2.6)

# Enable debug symbols by default
# must be done before project() statement
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

project(wavalyzer)

include_directories("${PROJECT_BINARY_DIR}")

# Define sources and executable
add_executable("wavalyzer"
    src/wavalyzer/main.cpp
    src/wavalyzer/wav.cpp
    src/wavalyzer/fft.cpp
    src/wavalyzer/window.cpp
    src/wavalyzer/gui.cpp
    src/wavalyzer/histogram.cpp
    src/wavalyzer/spectrogram.cpp
    src/wavalyzer/common.cpp
    src/wavalyzer/handler.cpp
    src/wavalyzer/sfml_pdf.cpp
)
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

add_executable("harmful"
    src/harmful/main.cpp
    src/harmful/wav.cpp
    src/harmful/synth.cpp
    src/harmful/parser.cpp
    src/harmful/sequencer.cpp
    src/harmful/common.cpp
)

# Detect and add SFML
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
find_package(SFML 2 REQUIRED network audio graphics window system)
find_package(LibHaru REQUIRED)
if(SFML_FOUND)
  include_directories(${SFML_INCLUDE_DIR})
  target_link_libraries("wavalyzer" ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
endif()

if (LIBHARU_FOUND)
    include_directories(${LIBHARU_INCLUDE_DIR})
    target_link_libraries("wavalyzer" ${LIBHARU_LIBRARIES})
endif()

# Install target
install(TARGETS "wavalyzer" DESTINATION bin)
