#pragma once

#include<vector>
#include"mesh.h"

#include"glm/glm.hpp"
#include"assimp/scene.h"

class MeshModel
{

public:


	MeshModel(std::vector<Mesh> NewMeshList);
	~MeshModel();

	size_t GetMeshCount();
	Mesh * GetMesh(size_t index);

	glm::mat4 & GetModel();
	void SetModel(glm::mat4 newModel);
	void DestroyMeshModel();

	static std::vector<std::vector<std::string>> LoadMaterials(const aiScene* scene);
	static std::vector<Mesh> LoadNode(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
		aiNode* node, const aiScene* scene, std::vector<int> matToTex);
	static Mesh LoadMesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
		aiMesh* mesh, const aiScene* scene, std::vector<int> matToTex);


private:
	std::vector<Mesh> m_MeshList;
	glm::mat4 model;

};

