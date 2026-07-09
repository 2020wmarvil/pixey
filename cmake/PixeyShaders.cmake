# Compile GLSL shaders to SPIR-V and embed them as C++ byte arrays.
#
# Usage:
#   pixey_compile_shaders(<target>
#       SHADERS <relative_or_abs_path>...
#   )
#
# Each shader produces a generated header at:
#   <binary_dir>/generated/Pixey/Shaders/<stem>_<stage>.spv.h
# Containing:
#   namespace Pixey::Shaders {
#       inline constexpr uint8_t <stem>_<stage>_spv[] = { ... };
#   }
#
# The generated include dir is added to <target> as PRIVATE.

function(pixey_compile_shaders TARGET)
	cmake_parse_arguments(ARG "" "" "SHADERS" ${ARGN})

	set(gen_include_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
	set(spv_out_dir     "${gen_include_dir}/Pixey/Shaders")
	file(MAKE_DIRECTORY "${spv_out_dir}")

	set(generated_headers "")

	foreach(shader_path IN LISTS ARG_SHADERS)
		get_filename_component(shader_abs  "${shader_path}" ABSOLUTE)
		get_filename_component(shader_name "${shader_path}" NAME)

		string(REPLACE "." "_" shader_id "${shader_name}")
		string(TOLOWER "${shader_id}" shader_id)

		set(spv_file    "${spv_out_dir}/${shader_id}.spv")
		set(header_file "${spv_out_dir}/${shader_id}.spv.h")

		add_custom_command(
			OUTPUT  "${spv_file}"
			COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
					-V "${shader_abs}"
					-o "${spv_file}"
			DEPENDS "${shader_abs}"
			COMMENT "GLSL -> SPIR-V: ${shader_name}"
			VERBATIM
		)

		add_custom_command(
			OUTPUT  "${header_file}"
			COMMAND ${CMAKE_COMMAND}
					-DSPV_FILE=${spv_file}
					-DHEADER_FILE=${header_file}
					-DSYMBOL=${shader_id}_spv
					-P "${CMAKE_SOURCE_DIR}/cmake/PixeyEmbedSpv.cmake"
			DEPENDS "${spv_file}"
					"${CMAKE_SOURCE_DIR}/cmake/PixeyEmbedSpv.cmake"
			COMMENT "Embedding ${shader_id}.spv"
			VERBATIM
		)

		list(APPEND generated_headers "${header_file}")
	endforeach()

	set(shaders_target "${TARGET}_shaders")
	add_custom_target(${shaders_target} DEPENDS ${generated_headers})
	add_dependencies(${TARGET} ${shaders_target})

	target_include_directories(${TARGET} PRIVATE "${gen_include_dir}")
endfunction()
