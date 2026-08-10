# Compile GLSL shaders to SPIR-V and embed them all as C++ byte arrays behind
# a single name-keyed lookup table.
#
# Usage:
#   pixey_compile_shaders(<target>
#       SHADERS <relative_or_abs_path>...
#   )
#
# Produces one generated header at:
#   <binary_dir>/generated/Pixey/Shaders/EmbeddedShaders.h
# Containing, per shader, an inline constexpr uint8_t byte array, plus:
#   namespace Pixey::Shaders
#   {
#       struct EmbeddedShader { std::string_view name; const uint8_t* data; size_t size; };
#       inline constexpr EmbeddedShader kEmbeddedShaders[] = { ... };
#       const EmbeddedShader* FindEmbeddedShader(std::string_view name);
#   }
# where `name` is the shader's filename (e.g. "gradient.comp"). Adding a new
# shader to SHADERS requires no matching C++ #include or symbol -- look it up
# by filename via FindEmbeddedShader at the call site.
#
# The generated include dir is added to <target> as PRIVATE.

function(pixey_compile_shaders TARGET)
	cmake_parse_arguments(ARG "" "" "SHADERS" ${ARGN})

	set(gen_include_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
	set(spv_out_dir     "${gen_include_dir}/Pixey/Shaders")
	file(MAKE_DIRECTORY "${spv_out_dir}")

	set(spv_files "")
	set(shader_names "")

	foreach(shader_path IN LISTS ARG_SHADERS)
		get_filename_component(shader_abs  "${shader_path}" ABSOLUTE)
		get_filename_component(shader_name "${shader_path}" NAME)

		string(REPLACE "." "_" shader_id "${shader_name}")
		string(TOLOWER "${shader_id}" shader_id)

		set(spv_file "${spv_out_dir}/${shader_id}.spv")

		add_custom_command(
			OUTPUT  "${spv_file}"
			COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
					-V "${shader_abs}"
					-o "${spv_file}"
			DEPENDS "${shader_abs}"
			COMMENT "GLSL -> SPIR-V: ${shader_name}"
			VERBATIM
		)

		list(APPEND spv_files "${spv_file}")
		list(APPEND shader_names "${shader_name}")
	endforeach()

	set(registry_header "${spv_out_dir}/EmbeddedShaders.h")

	add_custom_command(
		OUTPUT  "${registry_header}"
		COMMAND ${CMAKE_COMMAND}
				"-DSPV_FILES=${spv_files}"
				"-DSHADER_NAMES=${shader_names}"
				-DHEADER_FILE=${registry_header}
				-P "${CMAKE_SOURCE_DIR}/cmake/PixeyEmbedSpv.cmake"
		DEPENDS ${spv_files}
				"${CMAKE_SOURCE_DIR}/cmake/PixeyEmbedSpv.cmake"
		COMMENT "Embedding compiled shaders"
		VERBATIM
	)

	set(shaders_target "${TARGET}_shaders")
	add_custom_target(${shaders_target} DEPENDS "${registry_header}")
	add_dependencies(${TARGET} ${shaders_target})

	target_include_directories(${TARGET} PRIVATE "${gen_include_dir}")
endfunction()
