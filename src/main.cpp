#include <iostream>
#include <vulkan/vulkan.hpp>
#include "Window.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

int main(int argc, char** argv, char* envp[])
{
	std::cout << "hello world\n";
	Window wnd;
	wnd.create();
	uint32_t extensionCount = 0;
	vk::enumerateInstanceExtensionProperties();
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::cout << extensionCount << " extensions supported\n";

	glm::mat4 matrix;
	glm::vec4 vec;
	auto test = matrix * vec;
	wnd.destroy();
}