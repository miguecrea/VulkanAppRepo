#include "VulkanRenderer.h"
#include <stdexcept>

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

	vkDestroySurfaceKHR(m_Instance, surface, nullptr);
	vkDestroyDevice(mainDevice.logicalDevice, nullptr);
	if (enableValidationLayers)
	{
		DestroyDebugReportCallbackEXT(m_Instance, callback, nullptr);
	}
	vkDestroyInstance(m_Instance, nullptr);
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
	// 2 choose best presentation mode 
	// 3 choose swap chaion image resolution


}
