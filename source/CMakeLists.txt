project (Kaleido)

# 3rd party libs
link_directories(${DEPENDENCIES_DIRECTORY}/glad/lib)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "*.cpp")
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "*.h")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Kaleido" FILES ${HEADER_FILES} ${SOURCE_FILES})


if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/app/CMakeLists.txt" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/app/App.h")
	set(APP_DIR "app")
	message(STATUS "Detected app header at '${CMAKE_CURRENT_SOURCE_DIR}/app'. Using custom app files.") 
else()
	set(APP_DIR "app-default")
	message(STATUS "No specific app files detected. Using app-default.")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${APP_DIR})
include(${APP_DIR}/CMakeLists.txt)

add_executable(Kaleido ${SOURCE_FILES} ${HEADER_FILES})

target_precompile_headers(Kaleido PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/pch.h") 
 
if(MSVC)
	set_property(TARGET Kaleido PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()

# Vulkan
find_package(Vulkan REQUIRED)

# 3rd party libraries
target_link_libraries(Kaleido
	glad.lib
	opengl32.lib
	XInput.lib
	Vulkan::Vulkan
)

# glad dll
add_compile_definitions(GLAD_GLAPI_EXPORT)


# dependencies (build order)
add_dependencies(Kaleido 
	glad
)

# 3rd party includes
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY})
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/khr)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/nlohmann)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/ovr)

include_directories(${DEPENDENCIES_DIRECTORY})
include_directories(${DEPENDENCIES_DIRECTORY}/glm)
include_directories(${DEPENDENCIES_DIRECTORY}/spdlog/include)
include_directories(${DEPENDENCIES_DIRECTORY}/imgui)
include_directories(${DEPENDENCIES_DIRECTORY}/magic_enum/include)
include_directories(${DEPENDENCIES_DIRECTORY}/nlohmann/include)
include_directories(${DEPENDENCIES_DIRECTORY}/tinygltf)
include_directories(${DEPENDENCIES_DIRECTORY}/imfilebrowser)
include_directories(${DEPENDENCIES_DIRECTORY}/glad/include/glad)
include_directories(${DEPENDENCIES_DIRECTORY}/glad/include/KHR)
include_directories(${DEPENDENCIES_DIRECTORY}/glad/include/)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# for generated exports file
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# enforce cpp17
set_property(TARGET Kaleido PROPERTY CXX_STANDARD 17)
set_property(TARGET Kaleido PROPERTY CXX_STANDARD_REQUIRED ON)


# multi-core compilation
if(MSVC)
	target_compile_options(Kaleido PRIVATE "/MP")
	# unsafe strcpy used by some dependencies
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
