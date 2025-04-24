

cmake_minimum_required(VERSION 3.16)
project(VulkanApp)

set(CMAKE_CXX_STANDARD 17)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#exe has to be linked first 
add_executable(VulkanApp
  main.cpp
  VulkanRenderer.cpp
  Mesh.cpp
  MeshModel.cpp
)

#WHERE THE headers are 
target_include_directories(VulkanApp PRIVATE
  ${PROJECT_SOURCE_DIR}
)

# stb_image is just a header  I think what this does makes it so we dont ned to add the stdimage.h again ort something like that 
#I still manually copied the dll 
#target_compile_definitions(VulkanApp PRIVATE STB_IMAGE_IMPLEMENTATION)


#macrp top get the Location of the Root CmakeList.txt 
add_definitions(-DPROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")


include(FetchContent)

# GLFW
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw)

target_link_libraries(VulkanApp PRIVATE glfw)


# GLM (header-only)
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG master
)
FetchContent_MakeAvailable(glm)
target_link_libraries(VulkanApp PRIVATE glm)

FetchContent_Declare(
  assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG master
)
FetchContent_MakeAvailable(assimp)
target_link_libraries(VulkanApp PRIVATE assimp)

# link Vulkan

  find_package(Vulkan REQUIRED)
  target_link_libraries(VulkanApp PRIVATE Vulkan::Vulkan)





