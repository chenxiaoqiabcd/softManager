configure_file(
	${PROJECT_SOURCE_DIR}/downloadTool/downloadTool.rc.in 
	${PROJECT_SOURCE_DIR}/downloadTool/downloadTool.rc 
	@ONLY
)

file(GLOB COMMON_HEADER_FILES "../common/*.h")

source_group(common\\\\header_files FILES ${COMMON_HEADER_FILES})

file(GLOB COMMON_SOURCE_FILES "../common/*.cpp")

source_group(common\\\\source_files FILES ${COMMON_SOURCE_FILES})

file(GLOB skin_files "skin/*.xml")

source_group(skin FILES ${skin_files})

file(GLOB header_files "*.h")

source_group(header_files FILES ${header_files})

file(GLOB magick_hdrs "*.cpp")

source_group(source_files FILES ${magick_hdrs})

file(GLOB resource_files "downloadTool.rc")

source_group(resource_files FILES ${resource_files})

add_executable(downloadTool
	${skin_files}
	${header_files}
	${magick_hdrs}
	${resource_files}
	${COMMON_HEADER_FILES}
	${COMMON_SOURCE_FILES}
)

target_include_directories(downloadTool PRIVATE
	${PROJECT_SOURCE_DIR}/common
	${PROJECT_SOURCE_DIR}/third_party/DuiLib/src
	${PROJECT_SOURCE_DIR}/third_party/curl/include

	${jsoncons_SOURCE_DIR}/include
	${eventpp_SOURCE_DIR}/include
)

SET_TARGET_PROPERTIES(downloadTool PROPERTIES  LINK_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ignore:4099 /force")

SET_TARGET_PROPERTIES(downloadTool PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries(downloadTool
	duilib

	lunasvg
	msvcrt.lib
	
	debug ${PROJECT_SOURCE_DIR}/third_party/curl/debug/lib/libcurl-d.lib
	debug ${PROJECT_SOURCE_DIR}/third_party/curl/debug/lib/zlibd.lib

	optimized ${PROJECT_SOURCE_DIR}/third_party/curl/lib/libcurl.lib
	optimized ${PROJECT_SOURCE_DIR}/third_party/curl/lib/zlib.lib

	ws2_32.lib
	winmm.lib
	wldap32.lib
	Crypt32.lib
)

# add_custom_command(TARGET downloadTool POST_BUILD
#	COMMAND editbin /SUBSYSTEM:CONSOLE \"$(OutDir)$(ProjectName).exe\"
#	COMMENT "editbin successed!!!"
# )