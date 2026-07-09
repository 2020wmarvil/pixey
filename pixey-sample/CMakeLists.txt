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
