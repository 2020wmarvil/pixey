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
add_subdirectory(deps/VulkanMemoryAllocator EXCLUDE_FROM_ALL)

# GLM (header-only)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/glm EXCLUDE_FROM_ALL)

# stb (not a CMake project -- synthesize a header-only target)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE deps/stb)

# ---- Pixey static library --------------------------------------------------
add_library(pixey STATIC
	src/App.cpp
)

target_include_directories(pixey
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

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

# Exceptions: off by convention. Engine code must not throw.
# Not enforced at compile time because our third-party deps assume /EHsc and
# coordinating _HAS_EXCEPTIONS=0 across all TUs (incl. deps) is fragile.
# Revisit if/when hard enforcement is wanted.

# Shaders: GLSL -> SPIR-V -> embedded C++ header
include(PixeyShaders)
pixey_compile_shaders(pixey
	SHADERS
		shaders/sprite.vert
		shaders/sprite.frag
)

add_library(pixey::pixey ALIAS pixey)

# ---- Sample ----------------------------------------------------------------
if(PIXEY_BUILD_SAMPLE)
	add_subdirectory(pixey-sample)
	# Make F5 in VS run the sample by default instead of ALL_BUILD.
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		PROPERTY VS_STARTUP_PROJECT pixey-sample)
endif()
