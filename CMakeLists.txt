cmake_minimum_required(VERSION 3.28)

project(pixey
	VERSION 0.1.0
	DESCRIPTION "Game engine"
	LANGUAGES CXX
)

# ---- Top-level detection ---------------------------------------------------
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	set(PIXEY_IS_TOP_LEVEL ON)
else()
	set(PIXEY_IS_TOP_LEVEL OFF)
endif()

option(PIXEY_BUILD_SAMPLE
	"Build the in-tree pixey-sample dev driver"
	${PIXEY_IS_TOP_LEVEL})

option(PIXEY_SHIPPING
	"Build a shipping configuration (strips dev-only tooling)"
	OFF)

if(PIXEY_IS_TOP_LEVEL)
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ---- Vulkan SDK ------------------------------------------------------------
find_package(Vulkan REQUIRED COMPONENTS glslangValidator)

# ---- Third-party submodules ------------------------------------------------
# All deps under deps/ are expected to be populated git submodules.
# Target names below reflect upstream conventions; if any upstream renames a
# target, fix it here rather than papering over it elsewhere.

# SDL3 -- static, no tests
set(SDL_SHARED       OFF CACHE BOOL "" FORCE)
set(SDL_STATIC       ON  CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/SDL EXCLUDE_FROM_ALL)

# vk-bootstrap
add_subdirectory(deps/vk-bootstrap EXCLUDE_FROM_ALL)

# Vulkan Memory Allocator (header-only-ish)
# SYSTEM: its headers trip our /W4 /WX; exempt them rather than the whole project.
add_subdirectory(deps/VulkanMemoryAllocator EXCLUDE_FROM_ALL SYSTEM)

# GLM (header-only)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/glm EXCLUDE_FROM_ALL)

# stb (not a CMake project -- synthesize a header-only target)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE deps/stb)

# ---- Pixey dynamic library --------------------------------------------------
# src/ is globbed (CONFIGURE_DEPENDS) so new .h/.cpp files build without
# touching this file. include/Pixey stays explicit -- it's the small,
# stable public API surface, and PIXEY_API (PixeyExport.h) marks exactly what
# of it crosses the DLL boundary.
file(GLOB_RECURSE PIXEY_SOURCES CONFIGURE_DEPENDS
	src/*.h
	src/*.cpp
)

add_library(pixey SHARED
	${PIXEY_SOURCES}
	include/Pixey/App.h
	include/Pixey/EntryPoint.h
	include/Pixey/Input.h
	include/Pixey/Log.h
	include/Pixey/Pixey.h
	include/Pixey/PixeyExport.h
	include/Pixey/Types.h
	include/Pixey/Window.h
	include/Pixey/WindowManager.h
)

target_include_directories(pixey
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(pixey PRIVATE PIXEY_EXPORTS)

target_compile_features(pixey PUBLIC cxx_std_20)
set_target_properties(pixey PROPERTIES
	CXX_EXTENSIONS OFF
	FOLDER "pixey"
)

target_link_libraries(pixey
	PUBLIC
		Vulkan::Vulkan
		glm::glm
		SDL3::SDL3-static
	PRIVATE
		vk-bootstrap::vk-bootstrap
		GPUOpen::VulkanMemoryAllocator
		stb
)

if(MSVC)
	# /W4 /WX       -- strict warnings as errors (private; doesn't infect consumers)
	# /permissive-  -- modern standards conformance
	# /GR-          -- disable RTTI (style guide: no dynamic_cast / typeid)
	target_compile_options(pixey PRIVATE /W4 /WX /permissive- /GR-)
	target_compile_definitions(pixey PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

if(PIXEY_SHIPPING)
	target_compile_definitions(pixey PUBLIC PIXEY_SHIPPING)
else()
	# The Vulkan SDK only ships a release-CRT shaderc_combined (no debug variant),
	# which causes a RuntimeLibrary/_ITERATOR_DEBUG_LEVEL mismatch (LNK2038) when
	# statically linked into a Debug build. Link the shared (DLL) variant instead:
	# its object code doesn't get combined into our binary, so its CRT config
	# doesn't need to match ours. FindVulkan doesn't expose this as a component,
	# so it's wired up by hand from $ENV{VULKAN_SDK}.
	if(NOT DEFINED ENV{VULKAN_SDK})
		message(FATAL_ERROR "VULKAN_SDK environment variable is not set; required to locate shaderc_shared")
	endif()

	if(NOT TARGET shaderc::shaderc_shared)
		add_library(shaderc::shaderc_shared SHARED IMPORTED)
		set_target_properties(shaderc::shaderc_shared PROPERTIES
			IMPORTED_IMPLIB               "$ENV{VULKAN_SDK}/Lib/shaderc_shared.lib"
			IMPORTED_LOCATION              "$ENV{VULKAN_SDK}/Bin/shaderc_shared.dll"
			INTERFACE_INCLUDE_DIRECTORIES "$ENV{VULKAN_SDK}/Include"
		)
	endif()

	target_link_libraries(pixey PRIVATE shaderc::shaderc_shared)
	target_compile_definitions(pixey PRIVATE
		PIXEY_SHADER_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
	)
endif()

# Exceptions: off by convention. Engine code must not throw.
# Not enforced at compile time because our third-party deps assume /EHsc and
# coordinating _HAS_EXCEPTIONS=0 across all TUs (incl. deps) is fragile.
# Revisit if/when hard enforcement is wanted.

# Shaders: GLSL -> SPIR-V -> embedded C++ header
include(PixeyShaders)
file(GLOB PIXEY_SHADER_FILES CONFIGURE_DEPENDS
	shaders/*.vert
	shaders/*.frag
	shaders/*.comp
)
pixey_compile_shaders(pixey
	SHADERS
		${PIXEY_SHADER_FILES}
)

add_library(pixey::pixey ALIAS pixey)

# ---- Sample ----------------------------------------------------------------
if(PIXEY_BUILD_SAMPLE)
	add_subdirectory(pixey-sample)
	# Make F5 in VS run the sample by default instead of ALL_BUILD.
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		PROPERTY VS_STARTUP_PROJECT pixey-sample)
endif()
