#pragma once

//indices of locations of queue families
#include<fstream>
#define GLFW_INCLUDE_VULKAN
#include"GLFW/glfw3.h"
#include"glm.hpp"


const int MAX_FRAME_DRAWS = 2;

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};



struct Vertex
{
	glm::vec3 pos; //vertex position
	glm::vec3 col; //vertex position
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


static uint32_t FindMemoryTypeIndex(VkPhysicalDevice physicaldevice,uint32_t allowedTypes, VkMemoryPropertyFlags properties)
{
	// Get properties of physical device memory
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicaldevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((allowedTypes & (1 << i))														// Index of memory type must match corresponding bit in allowedTypes
			&& (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)	// Desired property bit flags are part of memory type's property flags
		{
			// This memory type is valid, so return its index
			return i;
		}
	}
}



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
	
	// Open stream from given file
	// std::ios::binary tells stream to read file as binary
	// std::ios::ate tells stream to start reading from end of file
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	// Check if file stream successfully opened
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open a file!");
	}

	// Get current read position and use to resize file buffer
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> fileBuffer(fileSize);

	// Move read position (seek to) the start of the file
	file.seekg(0);

	// Read the file data into the buffer (stream "fileSize" in total)
	file.read(fileBuffer.data(), fileSize);

	// Close stream
	file.close();

	return fileBuffer;

}

 
static void createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage,
	VkMemoryPropertyFlags bufferProperties, VkBuffer* buffer, VkDeviceMemory* bufferMemory)
{
	// CREATE VERTEX BUFFER
	   

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = bufferSize;		// Size of buffer (size of 1 vertex * number of vertices)
	bufferInfo.usage = bufferUsage;		// Multiple types of buffer possible, we want Vertex Buffer
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;			// Similar to Swap Chain images, can share vertex buffers

	VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr,buffer);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Vertex Buffer!");
	}

	// GET BUFFER MEMORY REQUIREMENTS
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

	// ALLOCATE MEMORY TO BUFFER
	VkMemoryAllocateInfo memoryAllocInfo = {};
	memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocInfo.allocationSize = memRequirements.size;

	memoryAllocInfo.memoryTypeIndex  = FindMemoryTypeIndex(physicalDevice, memRequirements.memoryTypeBits,bufferProperties);

	

   // Allocate memory to VkDeviceMemory

	result = vkAllocateMemory(device, &memoryAllocInfo, nullptr,bufferMemory);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Vertex Buffer Memory!");
	}

	// Allocate memory to given vertex buffer
	vkBindBufferMemory(device,*buffer,*bufferMemory,0);
}

static void copyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool,
	VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize)
{
	// Command buffer to hold transfer commands
	VkCommandBuffer transferCommandBuffer;

	// Command Buffer details
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = transferCommandPool;
	allocInfo.commandBufferCount = 1;

	// Allocate command buffer from pool
	vkAllocateCommandBuffers(device, &allocInfo, &transferCommandBuffer);

	// Information to begin the command buffer record
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;	// We're only using the command buffer once, so set up for one time submit

	// Begin recording transfer commands
	vkBeginCommandBuffer(transferCommandBuffer, &beginInfo);

	// Region of data to copy from and to
	VkBufferCopy bufferCopyRegion = {};
	bufferCopyRegion.srcOffset = 0;
	bufferCopyRegion.dstOffset = 0;
	bufferCopyRegion.size = bufferSize;

	// Command to copy src buffer to dst buffer
	vkCmdCopyBuffer(transferCommandBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);

	// End commands
	vkEndCommandBuffer(transferCommandBuffer);

	// Queue submission information  


	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &transferCommandBuffer;

	// Submit transfer command to transfer queue and wait until it finishes
	vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(transferQueue);

	// Free temporary command buffer back to pool
	vkFreeCommandBuffers(device, transferCommandPool, 1, &transferCommandBuffer);
}



