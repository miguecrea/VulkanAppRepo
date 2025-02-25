

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE  //DEPTH 0 to 1 

#include"GLFW/glfw3.h" 
#include"glm.hpp"
#include"mat4x4.hpp"
#include"iostream"


//glm THERE IS NO FOLDER BEFORE SO 


//libraries you eityher set them for 32 bit /64 bit or both  //AND DOWNLOAD TGHEM FOR that specific 

// additionla include directories 
//3 libraries included 
//GLM
//GLFW
// vulkaa header

// we need to set the path to where the lib file is for GLFW
//in the linker in general in additional library directories 

//and in the linker in input is which libaries do we actually wanna use so there we specify the lib file 



//GO to linker ->System ->Subsystem -> Set it to console 
//linker ystsem and subsystem shoulkd be in console

int main() 
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWwindow * window = glfwCreateWindow(800, 600, "Test Window",nullptr,nullptr);

	uint32_t extensionCount = 0;

	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	printf("ExtensionCount % i ", extensionCount);

	glm::mat4 testMatrix{ 4.f };
	glm::mat4 testMatrix2{ 4.f };

	auto gg = testMatrix * testMatrix2;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

	}
	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;


}