#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h" 
#include<vector>
#include"Utilities.h"

class VulkanRenderer
{

public:

	VulkanRenderer();
	~VulkanRenderer();

	int Init(GLFWwindow* window);
	void CleanUp();

	const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif


private:

	GLFWwindow * m_Window;
	VkInstance m_Instance;

	VkDebugReportCallbackEXT callback;

	struct 
	{
		VkPhysicalDevice physicaldevice;
		VkDevice  logicalDevice;
	} mainDevice;

	VkQueue graphicsQueue;

	VkSurfaceKHR surface;

	//functions
	void CreateInstance();
	void GetPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSurface();

	//- Support functions

	//check if we support certaij extensions
	bool checkIntanceExtensionsSupport(std::vector<const char*> * checkExtensions);
	bool CheckDeviceSuitable(VkPhysicalDevice device);

	//getters 

	QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device);
	bool checkValidationLayerSupport();
		
	void createDebugCallback();
	

};

