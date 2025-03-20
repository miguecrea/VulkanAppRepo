#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h" 

#include"glm.hpp"
#include"gtc/matrix_transform.hpp"
#include<vector>
#include"Utilities.h"
#include<set>
#include<algorithm>
#include"Mesh.h"

class VulkanRenderer
{

public:

	VulkanRenderer();
	~VulkanRenderer();

	void Draw();
	int Init(GLFWwindow* window);


	void UpdateModel(glm::mat4 newModel);

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

	//scene Objects 
	std::vector<Mesh> meshList;


	struct MVP
	{
		glm::mat4 projection; //how it sees 
		glm::mat4 view;  //camera
		glm::mat4 model;  //pos

	}mvp;

	VkDescriptorSetLayout descriptorSetLayout;

	std::vector<VkDescriptorSet> descriptorSets;

	VkDescriptorPool descriptorpool;

	std::vector<VkBuffer> uniformBuffer;
	std::vector<VkDeviceMemory> uniformBufferMemory;




	GLFWwindow * m_Window;
	VkInstance m_Instance;

	int currentFrame = 0;

	VkDebugReportCallbackEXT callback;

	struct 
	{
		VkPhysicalDevice physicaldevice;
		VkDevice  logicalDevice;
	} mainDevice;

	VkQueue graphicsQueue;
	VkQueue PresentationsQueue;

	VkSurfaceKHR surface;
	VkSwapchainKHR swapChain;

	//cached values

	VkFormat SwapChainImageFormat;
	VkExtent2D SwapChainExtend;


	std::vector<VkSemaphore>imageAvailable;  //signa when image is available 
	std::vector<VkSemaphore>RenderFinished; //signal when it ia finiahed rendering  and ready to present to screen
	std::vector<VkFence> drawFences; 



	//vector of images struct contains image and image views
	std::vector<SwapchainImage> swapChainImages;
	std::vector<VkFramebuffer> swapChainFrameBuffers;  //we gonna have a frame buffer for each image
	std::vector<VkCommandBuffer> commandBuffers;  


	//Pools
	VkCommandPool graphicsCommandPool;





	//pIpleine 
	VkPipelineLayout pipelineLayout;  
	VkRenderPass renderPass;
	VkPipeline graphicsPipeline;





	//functions
	void CreateInstance();
	void GetPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSurface();
	void createDebugCallback();
	void CreateSwapChain();

	void CreateRenderPass();
	void CreateDesciptorSetLayout();
	void CreateGraphicsPipeline();
	void CreateFrameBuffers();
	void CreateCommandPool();
	void CreateCommandBuffers();
	void RecordCommands();
	void CreateSynchronization();
	void CreateUniformBuffers();
	void CreateDesciptorPool();

	void CreateDescriptorSets();

	void UpdateUniformBuffers(uint32_t imageIndex);


	//- Support functions

	//check if we support certaij extensions
	bool checkIntanceExtensionsSupport(std::vector<const char*> * checkExtensions);
	bool CheckDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	bool checkValidationLayerSupport();

	//getters 

	QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device);
	SwapChainDetails GetSwapChainDetails(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR> & formats);
	VkPresentModeKHR ChooseBestPresentationMode(const std::vector<VkPresentModeKHR> & presentationModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);



		
	

};

