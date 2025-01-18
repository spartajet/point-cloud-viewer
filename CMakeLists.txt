cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0074 OLD)

project(PointCloudViewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_PREFIX_PATH "D:/cpp/VTK-9.4.1/build/lib/cmake/vtk-9.4" "C:/Qt/6.8.1/msvc2022_64")
set(Qt6_DIR "C:/Qt/6.8.1/msvc2022_64")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package (VTK REQUIRED)

IF (VTK_FOUND)
    MESSAGE(STATUS "VTK found.")
#    INCLUDE(${VTK_USE_FILE})
ENDIF()

find_package (PCL 1.14.1 REQUIRED)
include_directories(${PCL_INCLUDE_DIRS})
include_directories("D:/cpp/libs/lz4/include")
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

find_package(QT NAMES Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        pclvisualizer.cpp
        pclvisualizer.h
        pclvisualizer.ui
        # 测试
        inputdialog.h
        inputdialog.cpp
        inputdialog.ui
                images.qrc
		logo.rc

)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PointCloudViewer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET PointCloudViewer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(PointCloudViewer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(PointCloudViewer
           # 是否打开控制台
           # WIN32
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(PointCloudViewer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${PCL_LIBRARIES} ${VTK_LIBRARIES})

set_target_properties(PointCloudViewer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

vtk_module_autoinit(
        TARGETS PointCloudViewer
        MODULES ${VTK_LIBRARIES}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PointCloudViewer)
endif()
