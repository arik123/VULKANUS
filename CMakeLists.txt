cmake_minimum_required(VERSION 3.17)
project(VulkanTest1)
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(_DEBUG)
    if(MSVC)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -bigobj")
    endif()
endif ()
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(VulkanTest1 "src/main.cpp" "src/Window.h" "src/Window.cpp" "src/app.h")

target_include_directories(VulkanTest1 PRIVATE ${Vulkan_INCLUDE_DIRS})

target_link_libraries(VulkanTest1 PRIVATE glm)
TARGET_LINK_LIBRARIES(VulkanTest1 PUBLIC ${Vulkan_LIBRARIES})