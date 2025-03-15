cmake_minimum_required(VERSION 3.30)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0144 NEW)

project(PointCloudViewer VERSION 0.1 LANGUAGES CXX)

if (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    set(CMAKE_PREFIX_PATH "~/Qt/6.8.1/gcc_64")
else (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    set(VTK_DIR "D:/cpp/libs/vtk/lib/cmake/vtk-9.4")
    set(PCL_DIR "D:/cpp/libs/pcl/cmake")
    set(FLANN_ROOT "D:/cpp/libs/pcl/3rdParty/FLANN")
    set(PCL_ROOT "D:/cpp/libs/pcl")
    include_directories("D:/cpp/libs/lz4/include")
endif ()

#set(CMAKE_PREFIX_PATH "C:/Qt/6.8.1/msvc2022_64")


set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package (VTK REQUIRED)



find_package (PCL REQUIRED)
include_directories(${PCL_INCLUDE_DIRS})
message(STATUS "PCL_INCLUDE_DIRS: ${PCL_INCLUDE_DIRS}")

link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

find_package(QT NAMES Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package (VTK 9.4 REQUIRED)
IF (VTK_FOUND)
    MESSAGE(STATUS "VTK found.")
    #    INCLUDE(${VTK_USE_FILE})
ENDIF()

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
