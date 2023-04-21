cmake_minimum_required(VERSION 3.1)
project(df-announcements)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Protobuf REQUIRED)
find_package(Git)

option(USE_EXTERNAL_DFHACKCLIENTQT "Use dfhack-client-qt from external subdirectory" ON)
if(${USE_EXTERNAL_DFHACKCLIENTQT})
	add_subdirectory(external/dfhack-client-qt)
	add_library(DFHackClientQt::dfhack-client-qt ALIAS dfhack-client-qt)
else()
	find_package(DFHackClientQt REQUIRED)
endif()

add_custom_target(version ${CMAKE_COMMAND}
	-D GIT_EXECUTABLE=${GIT_EXECUTABLE}
	-D PROJECT_DIR=${CMAKE_CURRENT_SOURCE_DIR}
	-D VERSION_IN=${CMAKE_CURRENT_SOURCE_DIR}/src/Version.h.in
	-D VERSION_OUT=${CMAKE_CURRENT_BINARY_DIR}/Version.h
	-D FALLBACK_VERSION=1.0
	-P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GitVersion.cmake
)

set(CPP_SOURCES
	src/main.cpp
	src/AnnouncementTypeList.cpp
	src/Application.cpp
	src/ColorDelegate.cpp
	src/ColorPaletteModel.cpp
	src/GameManager.cpp
	src/MainWindow.cpp
	src/ReportFilterProxyModel.cpp
	src/ReportModel.cpp
	src/Settings.cpp
	src/SettingsDialog.cpp
)

qt6_wrap_ui(UI_SOURCES
	ui/AboutDialog.ui
	ui/MainWindow.ui
	ui/SettingsDialog.ui
)

set(PROTO_FILES
	proto/Reports.proto
)
protobuf_generate_cpp(PROTO_SOURCES PROTO_HEADERS ${PROTO_FILES})
set_property(SOURCE ${PROTO_SOURCES} ${PROTO_HEADERS} PROPERTY SKIP_AUTOMOC ON)

add_executable(df-announcements WIN32 ${CPP_SOURCES} ${UI_SOURCES} ${PROTO_SOURCES})
add_dependencies(df-announcements version)
target_include_directories(df-announcements PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(df-announcements
	Qt6::Widgets
	DFHackClientQt::dfhack-client-qt
	protobuf::libprotobuf
)
set_target_properties(df-announcements PROPERTIES
	AUTOMOC ON
)

