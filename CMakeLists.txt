cmake_minimum_required(VERSION 2.8)

project(snapDeformer)

include($ENV{DEVKIT_LOCATION}/cmake/pluginEntry.cmake)

set(PROJECT_NAME snapDeformer)

set(SOURCE_FILES
    snapDeformer.h
    snapDeformer.cpp
    mainPlugin.cpp

)

set(LIBRARIES
      OpenMaya
      Foundation
      OpenMayaAnim

)

build_plugin()
