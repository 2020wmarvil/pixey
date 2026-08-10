add_executable(pixey-sample
	src/Main.cpp
)

target_link_libraries(pixey-sample PRIVATE pixey::pixey)

set_target_properties(pixey-sample PROPERTIES
	FOLDER "pixey"
	VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:pixey-sample>"
)

if(MSVC)
	target_compile_options(pixey-sample PRIVATE /W4 /WX /permissive- /GR-)
endif()

# Copy assets next to the executable post-build.
add_custom_command(TARGET pixey-sample POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_CURRENT_SOURCE_DIR}/assets"
		"$<TARGET_FILE_DIR:pixey-sample>/assets"
	COMMENT "Copying pixey-sample assets"
)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/pixey.dll"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"$<TARGET_FILE:pixey>"
		"${CMAKE_CURRENT_BINARY_DIR}"
	DEPENDS pixey
	COMMENT "Copying pixey.dll"
	VERBATIM
)
add_custom_target(pixey_dll_copy ALL
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/pixey.dll"
)
add_dependencies(pixey-sample pixey_dll_copy)

if(NOT PIXEY_SHIPPING)
	# shaderc_shared.dll is loaded at runtime, not just link time.
    # copy it next to the exe so the dev build can actually launch.
	add_custom_command(TARGET pixey-sample POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"$ENV{VULKAN_SDK}/Bin/shaderc_shared.dll"
			"$<TARGET_FILE_DIR:pixey-sample>"
		COMMENT "Copying shaderc_shared.dll"
		VERBATIM
	)
endif()
