#include "Mesh.h"



Mesh::Mesh()
{
}

Mesh::Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices, std::vector<uint32_t> * indices)
{
	vertexCount = vertices->size();
	indexCount = indices->size();
	physicalDevice = newPhysicalDevice;
	device = newDevice;
	createVertexBuffer(transferQueue, transferCommandPool, vertices);
	createIndexBuffer(transferQueue, transferCommandPool, indices);
}

int Mesh::getVertexCount()
{
	return vertexCount;
}

VkBuffer Mesh::getVertexBuffer()
{
	return vertexBuffer;
}

int Mesh::getIndexCount()
{
	return indexCount;
}

VkBuffer Mesh::getIndexBuffer()
{
	return IndexBuffer;
}

void Mesh::destroyBuffers()
{
	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkFreeMemory(device, vertexBufferMemory, nullptr);
	vkDestroyBuffer(device,IndexBuffer, nullptr);
	vkFreeMemory(device, IndexBufferMemory, nullptr);
}


Mesh::~Mesh()
{
}

void Mesh::createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool,std::vector<Vertex>* vertices)
{
	VkDeviceSize BufferSize = sizeof(Vertex) * vertices->size();




	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;



	//we are gonna create a staging buffer ,tempoRALLY BUFFER 

	createBuffer(physicalDevice, device, BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	, &stagingBuffer, &stagingBufferMemory);







	// MAP MEMORY TO VERTEX BUFFER
	void* data;																// 1. Create pointer to a point in normal memory
	vkMapMemory(device, stagingBufferMemory, 0, BufferSize, 0, &data);		// 2. "Map" the vertex buffer memory to that point
	memcpy(data, vertices->data(), (size_t)BufferSize);					// 3. Copy memory from vertices vector to the point
	vkUnmapMemory(device, stagingBufferMemory);									// 4. Unmap the vertex buffer memory



	//CREATE BUFFER with transfer_dst_bit  TO Mark as recipient of transfer data

	//now  buffer memeory is device local bit  meaning is on the gpu and ONLY Accesible by it 


	//this is a GPU oNLY VERTEX BUFFER 
	createBuffer(physicalDevice, device, BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory);

	//copying from staging buffer to gpu only  buffer 
	copyBuffer(device, transferQueue, transferCommandPool, stagingBuffer, vertexBuffer, BufferSize);


	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);



}

void Mesh::createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices)
{
	VkDeviceSize bufferSize = sizeof(uint32_t) * indices->size();

	// Temporary buffer to "stage" index data before transferring to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

	// MAP MEMORY TO INDEX BUFFER
	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices->data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	// Create buffer for INDEX data on GPU access only area
	createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &IndexBuffer, &IndexBufferMemory);

	// Copy from staging buffer to GPU access buffer
	copyBuffer(device, transferQueue, transferCommandPool, stagingBuffer, IndexBuffer, bufferSize);

	// Destroy + Release Staging Buffer resources
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}


