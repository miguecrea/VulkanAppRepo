#include "MeshModel.h"



MeshModel::MeshModel(std::vector<Mesh> NewMeshList)
{
	m_MeshList = NewMeshList;
	model = glm::mat4(1.f);
}

MeshModel::~MeshModel()
{
}

size_t MeshModel::GetMeshCount()
{
	return m_MeshList.size();
}

Mesh * MeshModel::GetMesh(size_t index)
{
	if (index >= m_MeshList.size())
	{

		throw std::runtime_error("Attemted to acces in Invalid Index ");
	}

	return &m_MeshList[index];
}

glm::mat4 & MeshModel::GetModel()
{
	return model;
}

void MeshModel::SetModel(glm::mat4 newModel)
{
	model = newModel;
}

void MeshModel::DestroyMeshModel()
{
	for (auto & mesh : m_MeshList)
	{
		mesh.destroyBuffers();
	}
}


std::vector<std::vector<std::string>> MeshModel::LoadMaterials(const aiScene* scene)
{
	std::vector<std::vector<std::string>> TextureList(scene->mNumMaterials);  //materials present in Model

	printf(">>> number of mat: %d\n", scene->mNumMaterials);

	//go threougt each Material  and copy it is texture  file it it exist
	for (size_t i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* material = scene->mMaterials[i];

		//intitilaize the texture to empty string

		TextureList[i] = {};


		if (material->GetTextureCount(aiTextureType_DIFFUSE))
		{
			aiString path;
			//get the path of thbis texture 
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{
				//cut directory  if 
				int idx = std::string(path.data).rfind("\\");
				std::string FileName = std::string(path.data).substr(idx + 1);

				TextureList[i].push_back(FileName);
			}
		}
		else {
			TextureList[i].push_back("");
		}

		if (material->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS))
		{
			aiString path;
			//get the path of thbis texture 
			if (material->GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &path) == AI_SUCCESS)
			{
				//cut directory  if 

				int idx = std::string(path.data).rfind("\\");
				std::string FileName = std::string(path.data).substr(idx + 1);

				TextureList[i].push_back(FileName);
			}
		}
		else {
			TextureList[i].push_back("");
		}

		if (material->GetTextureCount(aiTextureType_NORMALS))
		{
			aiString path;
			//get the path of thbis texture 
			if (material->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS)
			{
				//cut directory  if 
				int idx = std::string(path.data).rfind("\\");
				std::string FileName = std::string(path.data).substr(idx + 1);
				TextureList[i].push_back(FileName);
			}
		}
		else {
			TextureList[i].push_back("");
		}

		if (material->GetTextureCount(aiTextureType_EMISSIVE))
		{
			aiString path;
			//get the path of thbis texture 
			if (material->GetTexture(aiTextureType_EMISSIVE, 0, &path) == AI_SUCCESS)
			{
				//cut directory  if 
				int idx = std::string(path.data).rfind("\\");
				std::string FileName = std::string(path.data).substr(idx + 1);
				TextureList[i].push_back(FileName);
			}
		}
		else {
			TextureList[i].push_back("");
		}
	}

	return TextureList;
}

std::vector<Mesh> MeshModel::LoadNode(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, aiNode * node, const aiScene* scene, std::vector<int> matToTex)
{
	
	std::vector<Mesh> meshList;

	// Go through each mesh at this node and create it, then add it to our meshList
	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		meshList.push_back(                                                          //gte the id of a particvular mesh in our node and use that id to get that mesh from our scene 
			LoadMesh(newPhysicalDevice, newDevice, transferQueue, transferCommandPool, scene->mMeshes[node->mMeshes[i]], scene, matToTex)
		);
	}



	// Go through each node attached to this node and load it, then append their meshes to this node's mesh list
	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		std::vector<Mesh> newList = LoadNode(newPhysicalDevice, newDevice, transferQueue, transferCommandPool, node->mChildren[i], scene, matToTex);
		meshList.insert(meshList.end(), newList.begin(), newList.end());
	}

	return meshList;
}

Mesh MeshModel::LoadMesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, aiMesh* mesh, const aiScene* scene, std::vector<int> matToTex)
{
	
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	// Resize vertex list to hold all vertices for mesh
	vertices.resize(mesh->mNumVertices);

	// Go through each vertex and copy it across to our vertices


	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		// Set position
		vertices[i].pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

		// Set tex coords (if they exist)
		if (mesh->mTextureCoords[0])
		{
			vertices[i].tex = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		}
		else
		{
			vertices[i].tex = { 0.0f, 0.0f };
		}

		vertices[i].normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

		// Set colour (just use white for now)
		vertices[i].col = { 1.0f, 1.0f, 1.0f };
	}





	// Iterate over indices through faces and copy across
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		// Get a face
		aiFace face = mesh->mFaces[i];

		// Go through face's indices and add to list
		for (size_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// Create new mesh with details and return it
	Mesh newMesh = Mesh(newPhysicalDevice, newDevice, transferQueue, transferCommandPool, &vertices, &indices, matToTex[mesh->mMaterialIndex]);

	return newMesh;
}
