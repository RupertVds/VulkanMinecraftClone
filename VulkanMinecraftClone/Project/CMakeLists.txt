# Source files
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

# Copy shaders
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${SHADER_BINARY_DIR}/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
)

# Copy textures
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/textures" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

set(SOURCES
    "main.cpp"
    "vulkanbase/VulkanBase.h" 
    "vulkanbase/VulkanUtil.h"
    "vulkanbase/VulkanUtil.cpp"
    # Add other source files here
    "labwork/Week01.cpp"
    "labwork/Week02.cpp" 
    "labwork/Week03.cpp" 
    "labwork/Week04.cpp" 
    "labwork/Week05.cpp"
    "labwork/Week06.cpp"
	"MachineShader.h" 
	"Mesh2D.h" "Mesh2D.cpp" 
	"CommandPool.h" "CommandPool.cpp" 
	"CommandBuffer.h" "CommandBuffer.cpp" 
	"QueueManager.h" "QueueManager.cpp" 
	"SwapchainManager.h" "SwapchainManager.cpp" 
	"vulkanbase/VulkanBase.cpp" 
	"Scene2D.h" "Scene2D.cpp"  
	"Scene.h" "Scene.cpp"
	"RenderPass.h" 
	"GraphicsPipeline.h"
	"BasicGraphicsPipeline2D.h" 
	"GraphicsPipeline3D.h" 
	"Camera.h" "Camera.cpp"
	"Timer.h" "Timer.cpp" 
	"InputManager.h" "InputManager.cpp" 
	"Game.h" "Game.cpp" 
	"Texture.h" "vendor/stb_image.h" "Texture.cpp"  "Block.h"  "BlockMeshGenerator.h" "BlockMeshGenerator.cpp" "vendor/json.hpp" "Chunk.h" "Chunk.cpp" "ChunkGenerator.h" "ChunkGenerator.cpp")

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${GLSL_SOURCE_FILES}  "BlockMesh.h" "BlockMesh.cpp")
add_dependencies(${PROJECT_NAME} Shaders)
# Link libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES} glfw)