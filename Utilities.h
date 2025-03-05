#pragma once

//indices of locations of queue families
#include<fstream>

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


struct QueueFamilyIndices
{

	//check if queue families are valid 
	int graphicsFamily = -1;
	int presentationFamily = -1;
	bool IsValid()
	{
		return graphicsFamily >= 0 && presentationFamily >=0;
	}
};


struct SwapChainDetails
{
	VkSurfaceCapabilitiesKHR surfacecapabilities;  //surface properties image size etc
	std::vector<VkSurfaceFormatKHR> formats;   // RGBA , and combinations of these 
	std::vector<VkPresentModeKHR> presentationModes;//how images should be presnted to screen 
};


struct SwapchainImage
{
	VkImage Image;
	VkImageView imageView;
};





static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags,				// Type of error
	VkDebugReportObjectTypeEXT objType,			// Type of object causing error
	uint64_t obj,								// ID of object
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* message,						// Validation Information
	void* userData)
{
	// If validation ERROR, then output error and return failure
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		printf("VALIDATION ERROR: %s\n", message);
		return VK_TRUE;
	}

	// If validation WARNING, then output warning and return okay
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		printf("VALIDATION WARNING: %s\n", message);
		return VK_FALSE;
	}

	return VK_FALSE;
}

static VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
	// vkGetInstanceProcAddr returns a function pointer to the requested function in the requested instance
	// resulting function is cast as a function pointer with the header of "vkCreateDebugReportCallbackEXT"
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

	// If function was found, executre if with given data and return result, otherwise, return error
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
	// get function pointer to requested function, then cast to function pointer for vkDestroyDebugReportCallbackEXT
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");

	// If function found, execute
	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
}

static std::vector<char> readFile(const std::string& filename)
{
	//read as binary and start from end 
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to opena  file");
	}

	size_t fileSize = { (size_t)file.tellg()};
	std::vector<char> fileBuffer(fileSize);


	//read from pos 0
	file.seekg(0);

	//read the file data into the buffer 

	file.read(fileBuffer.data(), fileSize);
	file.close();

}

 