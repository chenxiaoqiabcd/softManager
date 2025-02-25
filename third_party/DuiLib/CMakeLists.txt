cmake_minimum_required(VERSION 3.17)

add_link_options(/FORCE:MULTIPLE)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
add_definitions(-DUILIB_STATIC)
# add_definitions(-DUILIB_EXPORTS)

project(duilib)

include_directories(
	src
)

FILE(GLOB CORE_FILES "src/*.*" 
	"src/Control/*.*"
	"src/core/*.*"
	"src/Layout/*.*"
)

FILE(GLOB UTIL_FILES "src/Utils/*.h" "src/Utils/*.cpp")

source_group(util ${UTIL_FILES})

add_library(duilib STATIC ${CORE_FILES} ${UTIL_FILES})

set_target_properties(duilib PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set_target_properties(lunasvg PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries(duilib optimized lunasvg)

set(PCH_HEADER_FILE "${PROJECT_SOURCE_DIR}/src/stdafx.h")
set(PCH_SOURCE_FILE "${PROJECT_SOURCE_DIR}/src/stdafx.cpp")

#MACRO(USE_MSVC_PCH pdfTransformer PCH_HEADER_FILE PCH_SOURCE_FILE)
#IF(MSVC)
	# 获取预编译头文件的文件名，通常是stdafx
	# GET_FILENAME_COMPONENT(PCH_NAME ${PCH_HEADER_FILE} NAME_WE)
	
	# 生成预编译文件的路径
	# 如果有配置选项（Debug/Release），路径添加以及配置选项
    SET(PCH_DIR "${CMAKE_CURRENT_BINARY_DIR}/PCH/${CMAKE_CFG_INTDIR}")
 
	# 创建预编译文件的路径
	FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/PCH)
 
	# 设置项目属性，使用预编译头文件
	SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/Yu\"${PCH_HEADER_FILE}\" /FI\"${PCH_HEADER_FILE}\" /Fp\"$(IntDir)$(TargetName).pch\"")
 
	# 预编译源文件（stdafx.cpp）设置属性，创建预编译文件
	SET_SOURCE_FILES_PROPERTIES(${PCH_SOURCE_FILE} PROPERTIES COMPILE_FLAGS "/Yc\"${PCH_HEADER_FILE}\"")
    
	# 把预编译文件寄到清除列表
	SET_DIRECTORY_PROPERTIES(PROPERTIES
		ADDITIONAL_MAKE_CLEAN_FILES "$(IntDir)$(TargetName).pch")
#ENDIF(MSVC)
#ENDMACRO(USE_MSVC_PCH)


# FILE(GLOB TEST_FILES "test/*.h" "test/*.cpp")
# 
# add_executable(test ${TEST_FILES})
# 
# set_target_properties(test PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
# 
# SET_TARGET_PROPERTIES(test PROPERTIES LINK_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ignore:4099")
# 
# target_link_libraries(test
# 	duilib
# )