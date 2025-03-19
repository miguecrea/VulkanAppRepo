#include "VulkanRenderer.h"
#include <stdexcept>
#include<array>
VulkanRenderer::VulkanRenderer()
{
}

VulkanRenderer::~VulkanRenderer()
{
}

int VulkanRenderer::Init(GLFWwindow* window)
{
	m_Window = window;
	try
	{
		CreateInstance();
		createDebugCallback();
		CreateSurface();
		GetPhysicalDevice();
		CreateLogicalDevice();
		std::vector<Vertex> meshVertices = {
			{{0.4, -0.4, 0.0}, {1.0f, 0.0f, 0.0f}},
			{{0.4, 0.4, 0.0}, {0.0f, 1.0f, 0.0f}},
			{{-0.4, 0.4, 0.0}, {0.0f, 0.0f, 1.0f}},

			{ { -0.4, 0.4, 0.0 }, {0.0f, 0.0f, 1.0f}},
			{ { -0.4, -0.4, 0.0 }, {1.0f, 1.0f, 0.0f} },
			{ { 0.4, -0.4, 0.0 }, {1.0f, 0.0f, 0.0f} }
		};


		firstMesh = Mesh(mainDevice.physicaldevice, mainDevice.logicalDevice, &meshVertices);

		CreateSwapChain(); // swap bvetwen images and presentingto our surface 
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateFrameBuffers();
		CreateCommandPool();
		CreateCommandBuffers();
		RecordCommands();
		CreateSynchronization();

		
	}
	catch (const std::runtime_error& e)
	{
		printf("Error %s /n", e.what());

		return EXIT_FAILURE;
	}
	return 0;
}

void VulkanRenderer::CleanUp()
{

	//waiting Idle until no actions qre being perfromed 
	vkDeviceWaitIdle(mainDevice.logicalDevice);

	firstMesh.destroyVertexBuffer();

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		vkDestroySemaphore(mainDevice.logicalDevice, RenderFinished[i], nullptr);
		vkDestroySemaphore(mainDevice.logicalDevice, imageAvailable[i], nullptr);
		vkDestroyFence(mainDevice.logicalDevice, drawFences[i], nullptr);
	}
	 

	vkDestroyCommandPool(mainDevice.logicalDevice, graphicsCommandPool, nullptr);

	for (auto framebuffer : swapChainFrameBuffers)
	{
		vkDestroyFramebuffer(mainDevice.logicalDevice, framebuffer, nullptr);
	}

	vkDestroyPipeline(mainDevice.logicalDevice, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mainDevice.logicalDevice, pipelineLayout, nullptr);
	vkDestroyRenderPass(mainDevice.logicalDevice, renderPass, nullptr);

	for (auto image : swapChainImages)
	{

		vkDestroyImageView(mainDevice.logicalDevice, image.imageView, nullptr);

	}
	vkDestroySwapchainKHR(mainDevice.logicalDevice, swapChain, nullptr);
	vkDestroySurfaceKHR(m_Instance, surface, nullptr);
	vkDestroyDevice(mainDevice.logicalDevice, nullptr);
	if (enableValidationLayers)
	{
		DestroyDebugReportCallbackEXT(m_Instance, callback, nullptr);
	}
	vkDestroyInstance(m_Instance, nullptr);
}



void VulkanRenderer::Draw()
{


	//1 Get next avaiable image to draw , and signal when ot is finished drawing(semaphere)
	// 
	// 

	vkWaitForFences(mainDevice.logicalDevice, 1,&drawFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(mainDevice.logicalDevice, 1, &drawFences[currentFrame]);


	//use the semaphore so signal when image is ready 
	uint32_t imageIndex;
	vkAcquireNextImageKHR(mainDevice.logicalDevice, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);


	//wait for current fence to signal pen from last frame draw 

	// 2 submit command buffer to queue for executions, make sure image is ready before we actually draw
		// now signal when it is ready to be presented to the screen

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailable[currentFrame];				
	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};


	submitInfo.pWaitDstStageMask = waitStages;						// Stages to check semaphores at
	submitInfo.commandBufferCount = 1;								// Number of command buffers to submit
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];		// Command buffer to submit
	submitInfo.signalSemaphoreCount = 1;							// Number of semaphores to signal
	submitInfo.pSignalSemaphores = &RenderFinished[currentFrame];	                  // Semaphores to signal when command buffer finishes





	//signal this fence here 
	VkResult result = vkQueueSubmit(graphicsQueue,1, &submitInfo,drawFences[currentFrame]);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit Command Buffer to Queue!");
	}

	 


	// -- PRESENT RENDERED IMAGE TO SCREEN --
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;										// Number of semaphores to wait on
	presentInfo.pWaitSemaphores = &RenderFinished[currentFrame];			                //Semaphores to wait on
	presentInfo.swapchainCount = 1;											// Number of swapchains to present to
	presentInfo.pSwapchains = &swapChain;									// Swapchains to present images to
	presentInfo.pImageIndices = &imageIndex;								// Index of images in swapchains to present

	// Present image
	result = vkQueuePresentKHR(PresentationsQueue, &presentInfo);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present Image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAME_DRAWS;

}

void VulkanRenderer::CreateInstance()
{

	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	//Info about the app itself 
	//these dont affect the progra is for developer convenience
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan app";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); //custom version of the app
	appInfo.pEngineName = " No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;


	//creation info for a vulkan intance

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; //te stype we can guess it by the name
	createInfo.pApplicationInfo = &appInfo;

	//create list to hold intances extensions

	std::vector<const char*> InstanceExtensions = std::vector<const char*>();
	//set up extesnions intsance will use 
	uint32_t glfwExtensionsCount{ 0 };
	const char** glfwEntensions;  //array of pointers to c string


	glfwEntensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);


	for (uint32_t i = 0; i < glfwExtensionsCount; i++)
	{
		InstanceExtensions.push_back(glfwEntensions[i]);
	}

	if (enableValidationLayers)
	{
		InstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}



	if (!checkIntanceExtensionsSupport(&InstanceExtensions))
	{
		throw std::runtime_error(" vK instance does not support required extensions");
	}

	createInfo.enabledExtensionCount = static_cast<uint32_t>(InstanceExtensions.size());
	createInfo.ppEnabledExtensionNames = InstanceExtensions.data();


	//enabled validations layers 

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}

	//create intance

	VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance); //our instance 
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create a vulkan intance");
	}



}

void VulkanRenderer::GetPhysicalDevice()
{


	//enumertae physical devices 
	uint32_t deviceCount = 0;

	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);


	if (deviceCount == 0)
	{
		throw std::runtime_error("Cant Find GPUS that support Vulkan Intance");
	}

	std::vector<VkPhysicalDevice> deviceList(deviceCount);

	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, deviceList.data());

	for (const auto& device : deviceList)
	{
		if (CheckDeviceSuitable(device))
		{
			mainDevice.physicaldevice = device;
			break;
		}
	}

}

void VulkanRenderer::CreateLogicalDevice()
{
	//WHEn WE CREATER A logical device we need to say which queues we will need
	//queue the logical 

	//Get the index of queue family for the chosen device 
	QueueFamilyIndices Indices = GetQueueFamilies(mainDevice.physicaldevice);
	//get queue family gets te index of the family




	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	//if they somehow are the same the set makes sure there are no duplicates so
	//only one fo them is added 
	std::set<int> queueFamilyIndices = { Indices.graphicsFamily,Indices.presentationFamily };




	for (int QueueFamilyIndex : queueFamilyIndices)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = QueueFamilyIndex;
		queueCreateInfo.queueCount = 1;  //number of queues to create 
		float priority = 1.0f;
		queueCreateInfo.pQueuePriorities = &priority;   //if we have multiple queues runningnat once 
		//vulkan should now which one has the highest priority 


		queueCreateInfos.push_back(queueCreateInfo);

	}


	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());   //number of queue create info that one above
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();   // when it is singlre we use a & no we go
	//straight to that data



	//enabling swap chain extensions

	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());     //number of enabled logical device extensions
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();  //no extensions ye6

	//physical device features the logical device will be using 
	VkPhysicalDeviceFeatures deviceFeatures{};  //empty for now
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;


	//create a logical device 
	VkResult result = vkCreateDevice(mainDevice.physicaldevice, &deviceCreateInfo, nullptr, &mainDevice.logicalDevice);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a logical device ");
	}

	// index of the family   ,index 0 because we only create 1 queue for this family check above
	// and where to store the queue whichbis the grpahics one 

 //
	vkGetDeviceQueue(mainDevice.logicalDevice, Indices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(mainDevice.logicalDevice, Indices.presentationFamily, 0, &PresentationsQueue);

}

void VulkanRenderer::CreateSurface()
{

	//create surface 
	VkResult result = glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &surface);


	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a surface");
	}


}

bool VulkanRenderer::checkIntanceExtensionsSupport(std::vector<const char*>* checkExtensions)
{            //list of the names 

	//first get the amount of extensions

	uint32_t extensionCount = 0;
	//intance extensions there is also device but we checkingthe instance
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);


	//create a list of the size of extensioncount
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());



	//check if  given extensions are on th list 


	for (const auto& checkExensions : *checkExtensions)
	{
		bool HasExtension = false;

		for (const auto& Extensions : extensions)
		{
			if (strcmp(checkExensions, Extensions.extensionName))
			{
				HasExtension = true;
				break;
			}

		}

		if (!HasExtension)
		{
			return false;
		}

	}

	return true;
}

bool VulkanRenderer::CheckDeviceSuitable(VkPhysicalDevice device)
{


	////info about the device itself
	//VkPhysicalDeviceProperties deviceProperties;
	//vkGetPhysicalDeviceProperties(device, &deviceProperties);

 // //info about the device can do 
	//VkPhysicalDeviceFeatures deviceFeatures;
	//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);


	//check if queues are supported 


	QueueFamilyIndices indices = GetQueueFamilies(device);


	//check is sawp chain is supported 
	bool ExtensionsSupported = CheckDeviceExtensionSupport(device);

	bool IsSwapChainValid = false;
	if (ExtensionsSupported)
	{

		SwapChainDetails details{ GetSwapChainDetails(device) };
		IsSwapChainValid = !details.presentationModes.empty() && !details.formats.empty();
	}


	return indices.IsValid() && ExtensionsSupported && IsSwapChainValid;  //can onlky be valid if it supports all the queues families and extensions

}

bool VulkanRenderer::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{

	//check if swap chain is supported 


	uint32_t ExtensionsCount = 0;

	vkEnumerateDeviceExtensionProperties(device, nullptr, &ExtensionsCount, nullptr);


	if (ExtensionsCount == 0)
	{
		return false;
	}


	std::vector<VkExtensionProperties> Extensions(ExtensionsCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &ExtensionsCount, Extensions.data());



	for (const auto& deviceExtensions : deviceExtensions)  //this is the inside the utils 
	{
		bool HasExtension = false;

		for (const auto& extension : Extensions)
		{

			if (strcmp(deviceExtensions, extension.extensionName) == 0)
			{
				HasExtension = true;
				break;
			}

		}

		if (!HasExtension)
		{
			return false;
		}

	}


	return true;



}

QueueFamilyIndices VulkanRenderer::GetQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);

	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

	//go throught each queue family and check if it has at least 1 of the required types of queue
	// 
	int i = 0;

	for (const auto& queueFamily : queueFamilyList)
	{
		//IF THERE is at least 1 queue in the family then check what type of queue it is 
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 PresentationSupport = false;
		//If a particular queue family supports a surface
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &PresentationSupport);


		//check is queue type is grpahics and presentations
		if (queueFamily.queueCount > 0 && PresentationSupport)
		{
			indices.presentationFamily = i;

		}


		if (indices.IsValid())
		{
			break;
			//stop searching if found a valid one
		}
		i++;

	}


	return indices;



}

SwapChainDetails VulkanRenderer::GetSwapChainDetails(VkPhysicalDevice device)
{

	SwapChainDetails swapChainDetails = {};


	//get surface capabilities for the given surface
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainDetails.surfacecapabilities);

	//formats
	uint32_t formatCount{};
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);



	if (formatCount != 0)
	{
		swapChainDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapChainDetails.formats.data());
	}

	//presentationsMode
	uint32_t PresentationCount{};
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &PresentationCount, nullptr);


	if (PresentationCount != 0)
	{

		swapChainDetails.presentationModes.resize(PresentationCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &PresentationCount, swapChainDetails.presentationModes.data());

	}


	// create a optimL Swap chain for that surface 

	return swapChainDetails;
}

VkSurfaceFormatKHR VulkanRenderer::ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
	//so what is the best surface format ?  that is subjective 
	// ,format  VK_FORMAT_R8G8B8A8_UNORM
	//color space   VK_COLOR_SPACE_SRGB_NONLINEAR_KHR // color space does this color gamut, gamma correction, and white point of an image
	//chat GPT 
	//In Vulkan, a color space tells your computer how to display colors correctly on the screen. Different screens and devices interpret colors in different ways,
	// so Vulkan needs to know which color space to use to make sure the colors look right.



	//if there is only one
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_R8G8B8A8_UNORM,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}


	for (const auto& format : formats)
	{
		if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}



	return formats[0];



}

VkPresentModeKHR VulkanRenderer::ChooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentationModes)
{
	//this are the modes when swaping images some of them cause tearing 

	//lOOK For mailbox
	for (const auto& presentationMode : presentationModes)
	{
		if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR)  //MAILBOX where it waits I think
		{
			return presentationMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;

}

VkExtent2D VulkanRenderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
	//if is not tat value then the vaue already has what we want inside 
	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return  surfaceCapabilities.currentExtent;
	}
	else
	{
		int width{}, height{};
		glfwGetFramebufferSize(m_Window, &width, &height);
		VkExtent2D newExtend = {};
		newExtend.width = static_cast<uint32_t>(width);
		newExtend.height = static_cast<uint32_t>(height);


		//surface also defines max and mins so make sure it is in bounbdaries
		//clamping the values 

		newExtend.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtend.width));
		newExtend.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, newExtend.height));

		return newExtend;
	}
}

VkImageView VulkanRenderer::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{

	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image; // image to creaste view for 
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; //type of image 1d ,2d ,3d cube etc 
	viewCreateInfo.format = format;
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;   //WE CAN swizzle and get the r value to have same as the others 
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	//subResources  allow to view only a part of an image 
	viewCreateInfo.subresourceRange.aspectMask = aspectFlags;  //which aspect of image do you view
	viewCreateInfo.subresourceRange.baseMipLevel = 0;    //start mip map level to view rom
	viewCreateInfo.subresourceRange.levelCount = 1;    // how many mipmaps levels we want to view
	viewCreateInfo.subresourceRange.baseArrayLayer = 0; //start array level to view from
	viewCreateInfo.subresourceRange.layerCount = 1;  //number of array levels to view

	VkImageView imageView;

	VkResult result = vkCreateImageView(mainDevice.logicalDevice, &viewCreateInfo, nullptr, &imageView);


	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Image view");
	}

	return imageView;

}

VkShaderModule VulkanRenderer::CreateShaderModule(const std::vector<char> & code)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = code.size();
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());



	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(mainDevice.logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Shadr module\n");
	}

	return shaderModule;

}

bool VulkanRenderer::checkValidationLayerSupport()
{

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}
	return true;

}

void VulkanRenderer::createDebugCallback()
{

	if (!enableValidationLayers) return;

	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
	callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;	// Which validation reports should initiate callback
	callbackCreateInfo.pfnCallback = debugCallback;												// Pointer to callback function itself

	// Create debug callback with custom create function
	VkResult result = CreateDebugReportCallbackEXT(m_Instance, &callbackCreateInfo, nullptr, &callback);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Debug Callback!");
	}
}

void VulkanRenderer::CreateSwapChain()
{

	//first first get the details we need to the chain chain we create d a function  for that
	SwapChainDetails swapChainDetails = GetSwapChainDetails(mainDevice.physicaldevice);



	// 1  choose best surface format from that vector we have

	VkSurfaceFormatKHR surfaceformat = ChooseBestSurfaceFormat(swapChainDetails.formats);


	// 2 choose best presentation mode 


	VkPresentModeKHR presentMode = ChooseBestPresentationMode(swapChainDetails.presentationModes);

	// 3 choose swap chain image resolution
	VkExtent2D extend = ChooseSwapExtent(swapChainDetails.surfacecapabilities);



	//we want to use triple buffereing min is 2 
	uint32_t imageCount = swapChainDetails.surfacecapabilities.minImageCount + 1;


	//could be that the max is 0
	if (swapChainDetails.surfacecapabilities.maxImageCount > 0 && imageCount > swapChainDetails.surfacecapabilities.maxImageCount) //we overflowed
	{
		//we clamp the value to the max 
		imageCount = swapChainDetails.surfacecapabilities.maxImageCount;

	}

	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = surface;  //swap chainn surface 
	swapChainCreateInfo.imageFormat = surfaceformat.format;
	swapChainCreateInfo.imageColorSpace = surfaceformat.colorSpace;
	swapChainCreateInfo.presentMode = presentMode;
	swapChainCreateInfo.imageExtent = extend;
	swapChainCreateInfo.minImageCount = imageCount;
	swapChainCreateInfo.imageArrayLayers = 1;     //nuber of layers for each image in chain
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // what attacgment images will be used as
	swapChainCreateInfo.preTransform = swapChainDetails.surfacecapabilities.currentTransform; //transform to performmon swap chain images
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //how to handle blending images 
	swapChainCreateInfo.clipped = VK_TRUE;  //WHETHER To clip,parts of images not in view (eg behind another window )



	//GetQueueFamilyIndices

	QueueFamilyIndices indices = GetQueueFamilies(mainDevice.physicaldevice);


	// if grpahics ans presentation families are different the swap chain must let images be shared between families

	if (indices.graphicsFamily != indices.presentationFamily)
	{

		uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily,(uint32_t)indices.presentationFamily };

		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapChainCreateInfo.queueFamilyIndexCount = 2; //  number of queues to share images  between  -> graphics amd presentation
		swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices; //array of queues to share between
	}
	else
	{
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfo.queueFamilyIndexCount = 0;
		swapChainCreateInfo.pQueueFamilyIndices = nullptr;
	}


	//Useful when resizing 
	//when we resize we destroy the old one 
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;


	//create swap chain

	VkResult result = vkCreateSwapchainKHR(mainDevice.logicalDevice, &swapChainCreateInfo, nullptr, &swapChain);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a SwapChain");
	}

	//store form later reference
	SwapChainImageFormat = surfaceformat.format;
	SwapChainExtend = extend;



	//image views are interfaces to image since images are just data 

	uint32_t swapChainImageCount;

	vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapChain, &swapChainImageCount, nullptr);
	std::vector<VkImage> images(swapChainImageCount);

	vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapChain, &swapChainImageCount, images.data());

	for (VkImage image : images)
	{

		SwapchainImage  swapchainimage = {};
		swapchainimage.Image = image;
		//extra values we cached
		swapchainimage.imageView = CreateImageView(image, SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		//ADDED To spawn 
		swapChainImages.push_back(swapchainimage);

	}






}

void VulkanRenderer::CreateRenderPass()
{

	//RENDER Pass 
	//handles executions and outputs of a pipeline
	//can haver multiple smaller subpasses and each can use a different pipeline
	//subpass dependencies 
	//	define stages in the pipeline where transitionsneed to happen




	/*A render pass is like a step - by - step painting process for your game screen.Imagine you're painting a picture:

		First, you paint the background(sky, ground, etc.).
		Then, you add characters and objects on top.
		Finally, you add special effects like shadows or glowing lights.
		Each of these steps is a render pass in a game engine or graphics API like Vulkan.It tells the GPU what to draw and in what order, making sure everything looks right on screen.


  */



	//Render pass in  NUTSHELL

	VkAttachmentDescription colourAttachment = {};
	colourAttachment.format = SwapChainImageFormat;						// Format to use for attachment
	colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;					// Number of samples to write for multisampling
	colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;				// Describes what to do with attachment before rendering
	colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;			// Describes what to do with attachment after rendering
	colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	// Describes what to do with stencil before rendering
	colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	// Describes what to do with stencil after rendering




	// Framebuffer data will be stored as an image, but images can be given different data layouts
	// to give optimal use for certain operations


	colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// Image data layout before render pass starts
	colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		// Image data layout after render pass (to change to)



	VkAttachmentReference colourAttachmentReference = {};
	colourAttachmentReference.attachment = 0;
	colourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;		
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colourAttachmentReference;

	// Need to determine when layout transitions occur using subpass dependencies
	std::array<VkSubpassDependency, 2> subpassDependencies;

	// Conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// Transition must happen after...
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;						// Subpass index (VK_SUBPASS_EXTERNAL = Special value meaning outside of renderpass)
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;		// Pipeline stage
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;				// Stage access mask (memory access)
	// But must happen before...
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = 0;



	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;;
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = 0;

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colourAttachment;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	renderPassCreateInfo.pDependencies = subpassDependencies.data();

	VkResult result = vkCreateRenderPass(mainDevice.logicalDevice, &renderPassCreateInfo, nullptr, &renderPass);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Render Pass!");
	}

}

void VulkanRenderer::CreateGraphicsPipeline()
{

	auto vertexShaderCode = readFile("Shaders/vert.spv");
	auto fragmentShaderCode = readFile("Shaders/frag.spv");





	//create  shader module to link to graphics pipeline

	VkShaderModule vertexShaderModule = CreateShaderModule(vertexShaderCode);
	VkShaderModule fragmentShaderModule = CreateShaderModule(fragmentShaderCode);
	

	// -- SHADER STAGE CREATION INFORMATION --
	// 
	// 
	// Vertex Stage creation information
	VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
	vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;				// Shader Stage name
	vertexShaderCreateInfo.module = vertexShaderModule;						// Shader module to be used by stage
	vertexShaderCreateInfo.pName = "main";									// Entry point in to shader

	// Fragment Stage creation information
	VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {};
	fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;				// Shader Stage name
	fragmentShaderCreateInfo.module = fragmentShaderModule;						// Shader module to be used by stage
	fragmentShaderCreateInfo.pName = "main";									// Entry point in to shader




	// Put shader stage creation info in to array
	// Graphics Pipeline creation info requires array of shader stage creates
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };


	

	//how the data for a single vertex is 


	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;									// Can bind multiple streams of data, this defines which one
	bindingDescription.stride = sizeof(Vertex);						// Size of a single vertex object
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;		// How to move between data after each vertex.



	//Pos attribute
	std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions;

	attributeDescriptions[0].binding = 0;							// Which binding the data is at (should be same as above)
	attributeDescriptions[0].location = 0;							// Location in shader where data will be read from
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;	// Format the data will take (also helps define size of data)
	attributeDescriptions[0].offset = offsetof(Vertex, pos);		// Where this attribute is defined in the data for a single vertex

	//loc is what we chnage from the shader 

	// Colour Attribute
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, col);




	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};

	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription; //LIST OF VERTEX biding descriptions
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();








	//input assembly -- assemble vertex input shapes triangles 



		// -- INPUT ASSEMBLY --
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;		// Primitive type to assemble vertices as
	inputAssembly.primitiveRestartEnable = VK_FALSE;					// Allow overriding of "strip" topology to start new primitives
	//basiacally stop that strip of triangles and draw another one 


// -- VIEWPORT & SCISSOR --
// 
// if we want to draw from middle point to to somewhere else like in split screen 
	VkViewport viewport = {};
	viewport.x = 0.0f;									// x start coordinate
	viewport.y = 0.0f;									// y start coordinate
	viewport.width = (float)SwapChainExtend.width;		// width of viewport
	viewport.height = (float)SwapChainExtend.height;	// height of viewport
	viewport.minDepth = 0.0f;							// min framebuffer depth
	viewport.maxDepth = 1.0f;							// max framebuffer depth

	// basically  from what we drew which part we drawing 
	VkRect2D scissor = {};
	scissor.offset = { 0,0 };							// Offset to use region from
	scissor.extent = SwapChainExtend;					// Extent to describe region to use, starting at offset




	//create the viewport with the info above 

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;






	//this is so we can tell the pipeline not to bake something into the pipeline 
	//so we can change stuff


	// -- DYNAMIC STATES --
	// Dynamic states to enable
	//std::vector<VkDynamicState> dynamicStateEnables;
	//dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);	// Dynamic Viewport : Can resize in command buffer with vkCmdSetViewport(commandbuffer, 0, 1, &viewport);
	//dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);	// Dynamic Scissor	: Can resize in command buffer with vkCmdSetScissor(commandbuffer, 0, 1, &scissor);

	//// Dynamic State creation info
	//VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	//dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	//dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
	//dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();







	// -- RASTERIZER --
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;			// clip peoples in far plane like minecraft it cant see past some point
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;	// Whether to discard data and skip rasterizer. Never creates fragments, only suitable for pipeline without framebuffer output
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;	// How to handle filling points between vertices
	rasterizerCreateInfo.lineWidth = 1.0f;						// How thick lines should be when drawn
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;		// Which face of a tri to cull
	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;	// Winding to determine which side is front
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;			// Whether to add depth bias to fragments (good for stopping "shadow acne" in shadow mapping)


	// -- MULTISAMPLING --
	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
	multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;  //way of antialiasing  removed jagged edges 
	multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;					// Enable multisample shading or not
	multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;	// Number of samples to use per fragment


	// -- BLENDING --
	// Blending decides how to blend a new colour being written to a fragment, with the old value

	//imagine there is  a transparent material and there is something behind you need to blend colors




	// Blend Attachment State (how blending is handled)
	VkPipelineColorBlendAttachmentState colourState = {};
	colourState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT	// Colours to apply blending to  //so all of them
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colourState.blendEnable = VK_TRUE;													// Enable blending

	// Blending uses equation: (srcColorBlendFactor * new colour) colorBlendOp (dstColorBlendFactor * old colour)
	colourState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colourState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colourState.colorBlendOp = VK_BLEND_OP_ADD;

	// Summarised  (VK_BLEND_FACTOR_SRC_ALPHA * new colour) + (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * old colour)
	//			   (new colour alpha * new colour) + ((1 - new colour alpha) * old colour)

	colourState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colourState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colourState.alphaBlendOp = VK_BLEND_OP_ADD;
	// Summarised: (1 * new alpha) + (0 * old alpha) = new alpha






	VkPipelineColorBlendStateCreateInfo colourBlendingCreateInfo = {};
	colourBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colourBlendingCreateInfo.logicOpEnable = VK_FALSE;				// Alternative to calculations is to use logical operations
	colourBlendingCreateInfo.attachmentCount = 1;
	colourBlendingCreateInfo.pAttachments = &colourState;


	// -- PIPELINE LAYOUT (TODO: Apply Future Descriptor Set Layouts) --
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 0;
	pipelineLayoutCreateInfo.pSetLayouts = nullptr;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	// Create Pipeline Layout

	//pipleine related to all input 


	VkResult result = vkCreatePipelineLayout(mainDevice.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Pipeline Layout!");
	}


	// -- DEPTH STENCIL TESTING --
	// TODO: Set up depth stencil testing

	//program does not have any depth stencil yet 




	// -- GRAPHICS PIPELINE CREATION --
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;									// Number of shader stages
	pipelineCreateInfo.pStages = shaderStages;							// List of shader stages
	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;		// All the fixed function pipeline states
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
	pipelineCreateInfo.pColorBlendState = &colourBlendingCreateInfo;
	pipelineCreateInfo.pDepthStencilState = nullptr;
	pipelineCreateInfo.layout = pipelineLayout;							// Pipeline Layout pipeline should use
	pipelineCreateInfo.renderPass = renderPass;							// Render pass description the pipeline is compatible with
	pipelineCreateInfo.subpass = 0;										// Subpass of render pass to use with pipeline

	// Pipeline Derivatives : Can create multiple pipelines that derive from one another for optimisation
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;	// Existing pipeline to derive from...
	pipelineCreateInfo.basePipelineIndex = -1;				// or index of pipeline being created to derive from (in case creating multiple at once)

	// Create Graphics Pipeline
	result = vkCreateGraphicsPipelines(mainDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Graphics Pipeline!");
	}


	//destroy shader modules no longer needed after pipleine creation
	vkDestroyShaderModule(mainDevice.logicalDevice, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(mainDevice.logicalDevice, vertexShaderModule, nullptr);




}

void VulkanRenderer::CreateFrameBuffers()
{
	//Frame buffer is a  conection between images and render pass
	swapChainFrameBuffers.resize(swapChainImages.size());


	// create a framwe buffer for each image
	for (size_t i = 0; i < swapChainFrameBuffers.size(); i++)
	{


		//image view cmes from custom struct
		//that has image and image view 
		std::array<VkImageView, 1> attachments = {
			swapChainImages[i].imageView
		};

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = renderPass;										// Render Pass layout the Framebuffer will be used with
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();							// List of attachments 
		framebufferCreateInfo.width = SwapChainExtend.width;								// Framebuffer width
		framebufferCreateInfo.height = SwapChainExtend.height;								// Framebuffer height
		framebufferCreateInfo.layers = 1;													// Framebuffer layers

		VkResult result = vkCreateFramebuffer(mainDevice.logicalDevice, &framebufferCreateInfo, nullptr, &swapChainFrameBuffers[i]);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a Framebuffer!");
		}
	}





}

void VulkanRenderer::CreateCommandPool()
{
	//command pool for graphics family
	QueueFamilyIndices queueFamilyIndices = GetQueueFamilies(mainDevice.physicaldevice);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;	// Queue Family type that buffers from this command pool will use

	// Create a Graphics Queue Family Command Pool
	VkResult result = vkCreateCommandPool(mainDevice.logicalDevice, &poolInfo, nullptr, &graphicsCommandPool);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Command Pool!");
	}
}

void VulkanRenderer::CreateCommandBuffers()
{
	// Vulkan works by prerecording a group of commands and submitting all to the queue at once 
	//command buffers are allocated from a pool
	//synchronization is done so we dont access resources at the same time 


	commandBuffers.resize(swapChainFrameBuffers.size());

	//they already exist we jus allocating them 

	VkCommandBufferAllocateInfo cbAllocInfo = {};
	cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.commandPool = graphicsCommandPool;
	cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;	// VK_COMMAND_BUFFER_LEVEL_PRIMARY	: Buffer you submit directly to queue. Cant be called by other buffers.
	// VK_COMMAND_BUFFER_LEVEL_SECONARY	: Buffer can't be called directly. Can be called from other buffers via "vkCmdExecuteCommands" when recording commands in primary buffer
	cbAllocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

	// Allocate command buffers and place handles in array of buffers
	VkResult result = vkAllocateCommandBuffers(mainDevice.logicalDevice, &cbAllocInfo, commandBuffers.data());
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Command Buffers!");
	}

}

void VulkanRenderer::RecordCommands()
{


	//we need to 

	VkCommandBufferBeginInfo bufferBeginInfo = {};
	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;	// Buffer can be resubmitted when it has already been submitted 
	//basically 




	// Information about how to begin a render pass 
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;							// Render Pass to begin
	renderPassBeginInfo.renderArea.offset = { 0, 0 };						// Start point of render pass in pixels
	renderPassBeginInfo.renderArea.extent = SwapChainExtend;				// Size of region to run render pass on (starting at offset)


	//Background
	VkClearValue clearValues[] = {
		{0.9f, 0.7f, 0.5, 1.0f}
	};
	renderPassBeginInfo.pClearValues = clearValues;							// List of clear values (TODO: Depth Attachment Clear Value)
	renderPassBeginInfo.clearValueCount = 1;


	//they will all draw toa  diufferent commands buffer 

	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		renderPassBeginInfo.framebuffer = swapChainFrameBuffers[i]; //frame buffer



		// Start recording commands to command buffer!

		VkResult result = vkBeginCommandBuffer(commandBuffers[i], &bufferBeginInfo);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to start recording a Command Buffer!");
		}

		// Begin Render Pass
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Bind Pipeline to be used in render pass
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);


		VkBuffer vertexBuffers[] = { firstMesh.getVertexBuffer() };					// Buffers to bind
		VkDeviceSize offsets[] = { 0 };												// Offsets into buffers being bound
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);	// Command to bind vertex buffer before drawing with them



		// Execute pipeline    3 times goes over the vertex shader and change the index
		vkCmdDraw(commandBuffers[i],static_cast<uint32_t>(firstMesh.getVertexCount()), 1, 0, 0);  //second index is how many times to draw it 

		// End Render Pass
		vkCmdEndRenderPass(commandBuffers[i]);

		// Stop recording to command bufferk
		result = vkEndCommandBuffer(commandBuffers[i]);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to stop recording a Command Buffer!");
		}
	}
}

void VulkanRenderer::CreateSynchronization()
{
	
	imageAvailable.resize(MAX_FRAME_DRAWS);
	RenderFinished.resize(MAX_FRAME_DRAWS);
	drawFences.resize(MAX_FRAME_DRAWS);

	// Semaphore creation information
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Fence creation information
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		if (vkCreateSemaphore(mainDevice.logicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailable[i]) != VK_SUCCESS ||
			vkCreateSemaphore(mainDevice.logicalDevice, &semaphoreCreateInfo, nullptr, &RenderFinished[i]) != VK_SUCCESS ||
			vkCreateFence(mainDevice.logicalDevice, &fenceCreateInfo, nullptr, &drawFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a Semaphore and/or Fence!");
		}
	}
}
