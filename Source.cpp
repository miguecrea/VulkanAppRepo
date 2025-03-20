



#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE  //DEPTH 0 to 1 
#include "GLFW/glfw3.h" 



#include <stdexcept>
#include<vector>
#include"iostream"
#include "VulkanRenderer.h"


GLFWwindow* m_Window;
VulkanRenderer m_VulkanRenderer;



void InitWindow(std::string name = "Miguel Lozano Vulkan Project", const int width = 800, const int height = 600)
{
	//initialize glfw
	glfwInit();

	//Set glfw to not work with OpneGl
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//not resizable
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_Window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);

}

int main()
{

	InitWindow();

	if (m_VulkanRenderer.Init(m_Window) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}



	float angle = 0.0f;
	float deltaTime = 0.0f;
	float LastTime = 0.0f;


	while (!glfwWindowShouldClose(m_Window))
	{
		glfwPollEvents();


		float now = glfwGetTime();
		deltaTime = now - LastTime;
		LastTime = now;

		angle += 10.f * deltaTime;
		if (angle > 360.f) { angle -= 360.f; }

		
		m_VulkanRenderer.UpdateModel(glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)));


		m_VulkanRenderer.Draw();
	}


	m_VulkanRenderer.CleanUp();
	//destroy GLFW
	glfwDestroyWindow(m_Window);
	glfwTerminate();
	
}


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