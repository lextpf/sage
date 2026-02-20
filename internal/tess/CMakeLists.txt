cmake_minimum_required(VERSION 3.15)
project(webcam_ocr LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

option(TESS_BUILD_WEBCAM_APP "Build webcam_ocr executable (requires OpenCV)" ON)
option(TESS_OCR_ENABLE_WEBCAM "Enable in-process webcam capture in tess_ocr (requires OpenCV)" ON)

if(TESS_BUILD_WEBCAM_APP OR TESS_OCR_ENABLE_WEBCAM)
    find_package(OpenCV REQUIRED)
endif()

if(TESS_BUILD_WEBCAM_APP)
    add_executable(webcam_ocr main.cpp)
    target_link_libraries(webcam_ocr PRIVATE ${OpenCV_LIBS})
endif()

set(TESS_OCR_SOURCES tess_ocr_api.cpp)
if(TESS_OCR_ENABLE_WEBCAM)
    list(APPEND TESS_OCR_SOURCES main.cpp)
endif()

add_library(tess_ocr STATIC ${TESS_OCR_SOURCES})
if(TESS_OCR_ENABLE_WEBCAM)
    target_compile_definitions(tess_ocr PRIVATE TESS_OCR_NO_MAIN TESS_OCR_HAVE_INPROC_WEBCAM)
    target_link_libraries(tess_ocr PUBLIC ${OpenCV_LIBS})
endif()
target_include_directories(tess_ocr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(TESS_BUILD_WEBCAM_APP)
    add_custom_command(TARGET webcam_ocr POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/ocr.py
                $<TARGET_FILE_DIR:webcam_ocr>/ocr.py)
endif()

add_custom_command(TARGET tess_ocr POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/ocr.py
            $<TARGET_FILE_DIR:tess_ocr>/ocr.py)
