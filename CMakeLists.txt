cmake_minimum_required(VERSION 3.22)

project(TilesetCreator LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXTERNAL_DIR "$ENV{External}")

list(APPEND CMAKE_PREFIX_PATH "D:/svn/External/multiplatform/install/windows/qt")

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION src/main.cpp src/Image.cpp src/ImageViewer.cpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE -D_Qt_DEFINES_)

set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON)

target_include_directories(${PROJECT_NAME} PRIVATE
	"${EXTERNAL_DIR}/stb"
	"${CMAKE_SOURCE_DIR}/src")

target_link_libraries(${PROJECT_NAME} PRIVATE
	Qt6::Widgets)

if(WIN32)
#   Post-build launching of the windeployqt utilit (win platform only)
	get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
	get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
	find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#   QtDeploy
		COMMAND "${CMAKE_COMMAND}" -E env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:${PROJECT_NAME}>" COMMENT "Running windeployqt...")
endif()

install(TARGETS ${PROJECT_NAME}
	BUNDLE DESTINATION .
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

qt_finalize_executable(${PROJECT_NAME})
	
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>/res)