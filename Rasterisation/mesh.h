#pragma once
#include "mathLib.h"
#include <d3d11.h>
#include <corecrt_math_defines.h>
#include "GEMLoader.h"
#include "animation.h"
#include "dxCore.h"
#include "shader.h"
#include "texture.h"
#include "collision.h"

struct STATIC_VERTEX
{
	mathLib::Vec3 pos;
	mathLib::Vec3 normal;
	mathLib::Vec3 tangent;
	float tu;
	float tv;
};

struct ANIMATED_VERTEX
{
	mathLib::Vec3 pos;
	mathLib::Vec3 normal;
	mathLib::Vec3 tangent;
	float tu;
	float tv;
	unsigned int bonesIDs[4];
	float boneWeights[4];
};


struct Vertex
{
	// screen space position and colour
	mathLib::Vec3 position;
	mathLib::Color color;
};

static STATIC_VERTEX addVertex(mathLib::Vec3 p, mathLib::Vec3 n, float tu, float tv)
{
	STATIC_VERTEX v;
	v.pos = p;
	v.normal = n;
	//Frame frame;
	//frame.fromVector(n);
	//v.tangent = frame.u; // For now
	v.tangent = mathLib::Vec3(1, 0, 0);
	v.tu = tu;
	v.tv = tv;
	return v;
}

class Triangle {
	Vertex vertices[3];
public:
	ID3D11Buffer* vertexBuffer;

	// create a triangle
	void Init() {
		vertices[0].position = mathLib::Vec3(0, 1.0f, 0);
		vertices[0].color = mathLib::Color(0, 1.0f, 0);
		vertices[1].position = mathLib::Vec3(-1.0f, -1.0f, 0);
		vertices[1].color = mathLib::Color(1.0f, 0, 0);
		vertices[2].position = mathLib::Vec3(1.0f, -1.0f, 0);
		vertices[2].color = mathLib::Color(0, 0, 1.0f);
	}

	// create buffer and upload vertices to GPU
	void createBuffer(ID3D11Device* device, int N = 3) {
		D3D11_BUFFER_DESC bd;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA uploadData;
		bd.ByteWidth = sizeof(Vertex) * N;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		uploadData.pSysMem = vertices;
		uploadData.SysMemPitch = 0;
		uploadData.SysMemSlicePitch = 0;
		device->CreateBuffer(&bd, &uploadData, &vertexBuffer);
	}

	// ask the GPU to draw a triangle
	void draw(ID3D11DeviceContext* devicecontext) {
		UINT offsets;
		offsets = 0;
		UINT strides = sizeof(Vertex); // size of vertex
		devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		devicecontext->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offsets);
		devicecontext->Draw(3, 0);
	}
};

class Mesh {
public:
	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* vertexBuffer;
	int indicesSize;
	UINT strides;
	std::vector<ANIMATED_VERTEX> animatedVertices;
	std::vector<STATIC_VERTEX> staticVertices;

	void init(DxCore* core, void* vertices, int vertexSizeInBytes, int numVertices, unsigned int* indices, int numIndices) {
		D3D11_BUFFER_DESC bd;
		memset(&bd, 0, sizeof(D3D11_BUFFER_DESC));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(unsigned int) * numIndices;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		D3D11_SUBRESOURCE_DATA data;
		memset(&data, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		data.pSysMem = indices;
		core->device->CreateBuffer(&bd, &data, &indexBuffer);
		bd.ByteWidth = vertexSizeInBytes * numVertices;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		data.pSysMem = vertices;
		core->device->CreateBuffer(&bd, &data, &vertexBuffer);
		indicesSize = numIndices;
		strides = vertexSizeInBytes;
	}

	void init(DxCore* core, std::vector<STATIC_VERTEX> vertices, std::vector<unsigned int> indices)
	{
		staticVertices = vertices;
		init(core, &vertices[0], sizeof(STATIC_VERTEX), vertices.size(), &indices[0], indices.size());
	}

	void init(DxCore* core, std::vector<ANIMATED_VERTEX> vertices, std::vector<unsigned int> indices)
	{
		animatedVertices = vertices;
		init(core, &vertices[0], sizeof(ANIMATED_VERTEX), vertices.size(), &indices[0], indices.size());
	}


	void draw(DxCore* core) {
		UINT offsets = 0;
		core->devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		core->devicecontext->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offsets);
		core->devicecontext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		core->devicecontext->DrawIndexed(indicesSize, 0, 0);
	}
};

class plane {
public:
	Mesh mesh;

	void init(DxCore* core) {
		std::vector<STATIC_VERTEX> vertices;
		std::vector<unsigned int> indices;
		int gridSize = 10;
		float planeSize = 100.0f;
		float step = planeSize * 2.0f / gridSize;

		float textureScale = 10.0f; // Control the proportion of texture repeats
		// Generate Vertices
		for (int z = 0; z <= gridSize; ++z) {
			for (int x = 0; x <= gridSize; ++x) {
				float posX = -planeSize + x * step;
				float posZ = -planeSize + z * step;
				float u = static_cast<float>(x) / gridSize * textureScale;
				float v = static_cast<float>(z) / gridSize * textureScale;
				vertices.push_back(addVertex(mathLib::Vec3(posX, 0.0f, posZ), mathLib::Vec3(0, 1, 0), u, v));
			}
		}

		// Generate indices
		for (int z = 0; z < gridSize; ++z) {
			for (int x = 0; x < gridSize; ++x) {
				int topLeft = z * (gridSize + 1) + x;
				int topRight = topLeft + 1;
				int bottomLeft = (z + 1) * (gridSize + 1) + x;
				int bottomRight = bottomLeft + 1;

				indices.push_back(topLeft);
				indices.push_back(bottomLeft);
				indices.push_back(topRight);

				indices.push_back(topRight);
				indices.push_back(bottomLeft);
				indices.push_back(bottomRight);
			}
		}
		mesh.init(core, vertices, indices);
	}

	// ask the GPU to draw a plane
	void draw(DxCore* core, Shader* shader, textureManager textures, sampler sam, mathLib::Matrix worldMatrix, mathLib::Matrix vp) {
		shader->updateConstantVS("staticMeshBuffer", "W", &worldMatrix);
		shader->updateConstantVS("staticMeshBuffer", "VP", &vp);
		shader->apply(core);
		shader->updateTexturePS(core, "tex", textures.find("Textures/grass.png"), sam.state);
		shader->updateTexturePS(core, "normalMap", textures.find("Textures/grass_Normal.png"), sam.state);
		mesh.draw(core);
	}
};

class cube {
public:
	Mesh mesh;
	AABB boundingBox;
	std::vector<STATIC_VERTEX> vertices;
	mathLib::Matrix worldMatrix;

	void init(DxCore* core) {
		mathLib::Vec3 p0 = mathLib::Vec3(-1.0f, -1.0f, -1.0f);
		mathLib::Vec3 p1 = mathLib::Vec3(1.0f, -1.0f, -1.0f);
		mathLib::Vec3 p2 = mathLib::Vec3(1.0f, 1.0f, -1.0f);
		mathLib::Vec3 p3 = mathLib::Vec3(-1.0f, 1.0f, -1.0f);
		mathLib::Vec3 p4 = mathLib::Vec3(-1.0f, -1.0f, 1.0f);
		mathLib::Vec3 p5 = mathLib::Vec3(1.0f, -1.0f, 1.0f);
		mathLib::Vec3 p6 = mathLib::Vec3(1.0f, 1.0f, 1.0f);
		mathLib::Vec3 p7 = mathLib::Vec3(-1.0f, 1.0f, 1.0f);

		vertices.push_back(addVertex(p0, mathLib::Vec3(0.0f, 0.0f, -1.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p1, mathLib::Vec3(0.0f, 0.0f, -1.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p2, mathLib::Vec3(0.0f, 0.0f, -1.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p3, mathLib::Vec3(0.0f, 0.0f, -1.0f), 0.0f, 0.0f));

		vertices.push_back(addVertex(p5, mathLib::Vec3(0.0f, 0.0f, 1.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p4, mathLib::Vec3(0.0f, 0.0f, 1.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p7, mathLib::Vec3(0.0f, 0.0f, 1.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p6, mathLib::Vec3(0.0f, 0.0f, 1.0f), 0.0f, 0.0f));

		vertices.push_back(addVertex(p4, mathLib::Vec3(-1.0f, 0.0f, 0.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p0, mathLib::Vec3(-1.0f, 0.0f, 0.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p3, mathLib::Vec3(-1.0f, 0.0f, 0.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p7, mathLib::Vec3(-1.0f, 0.0f, 0.0f), 0.0f, 0.0f));

		vertices.push_back(addVertex(p1, mathLib::Vec3(1.0f, 0.0f, 0.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p5, mathLib::Vec3(1.0f, 0.0f, 0.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p6, mathLib::Vec3(1.0f, 0.0f, 0.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p2, mathLib::Vec3(1.0f, 0.0f, 0.0f), 0.0f, 0.0f));

		vertices.push_back(addVertex(p3, mathLib::Vec3(0.0f, 1.0f, 0.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p2, mathLib::Vec3(0.0f, 1.0f, 0.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p6, mathLib::Vec3(0.0f, 1.0f, 0.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p7, mathLib::Vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f));

		vertices.push_back(addVertex(p4, mathLib::Vec3(0.0f, -1.0f, 0.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p5, mathLib::Vec3(0.0f, -1.0f, 0.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p1, mathLib::Vec3(0.0f, -1.0f, 0.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p0, mathLib::Vec3(0.0f, -1.0f, 0.0f), 0.0f, 0.0f));


		std::vector<unsigned int> indices;
		indices.push_back(0); indices.push_back(1); indices.push_back(2);
		indices.push_back(0); indices.push_back(2); indices.push_back(3);
		indices.push_back(4); indices.push_back(5); indices.push_back(6);
		indices.push_back(4); indices.push_back(6); indices.push_back(7);
		indices.push_back(8); indices.push_back(9); indices.push_back(10);
		indices.push_back(8); indices.push_back(10); indices.push_back(11);
		indices.push_back(12); indices.push_back(13); indices.push_back(14);
		indices.push_back(12); indices.push_back(14); indices.push_back(15);
		indices.push_back(16); indices.push_back(17); indices.push_back(18);
		indices.push_back(16); indices.push_back(18); indices.push_back(19);
		indices.push_back(20); indices.push_back(21); indices.push_back(22);
		indices.push_back(20); indices.push_back(22); indices.push_back(23);

		mesh.init(core, vertices, indices);

		for (auto& vertex : vertices) {
			boundingBox.extend(vertex.pos);
		}
	}

	void updateBoundingBox(mathLib::Matrix& worldMatrix) {
		boundingBox.reset();
		for (auto& vertex : vertices) {
			mathLib::Vec3 transformedPos = worldMatrix.mulPoint(vertex.pos); // 应用变换矩阵
			boundingBox.extend(transformedPos);
		}
	}

	// ask the GPU to draw a cube
	void draw(DxCore* core, Shader* shader, textureManager textures, sampler sam, mathLib::Matrix& worldMatrix, mathLib::Matrix& vp) {
		shader->updateConstantVS("staticMeshBuffer", "W", &worldMatrix);
		shader->updateConstantVS("staticMeshBuffer", "VP", &vp);
		shader->apply(core);
		shader->updateTexturePS(core, "tex", textures.find("Textures/Bricks097_1K-PNG_Color.png"), sam.state);
		shader->updateTexturePS(core, "normalMap", textures.find("Textures/Bricks097_1K-PNG_NormalDX.png"), sam.state);
		mesh.draw(core);
	}
};

class Pool {
public:
	std::vector<cube> cubes; // cubes
	mathLib::Vec3 poolSize;  // pool size
	float cubeSize;          // single cube size

	void init(DxCore* core, mathLib::Vec3 size, float cubeSize) {
		this->poolSize = size;
		this->cubeSize = cubeSize;

		// generate four sides
		generateWall(core, mathLib::Vec3(-poolSize.x / 2.0f, 1.0f, -poolSize.x / 2.0f), true);  // left
		generateWall(core, mathLib::Vec3(poolSize.x / 2.0f, 1.0f, -poolSize.x / 2.0f), true);   // right
		generateWall(core, mathLib::Vec3(-poolSize.x / 2.0f, 1.0f, poolSize.z / 2.0f), false); // front
		generateWall(core, mathLib::Vec3(-poolSize.x / 2.0f, 1.0f, -poolSize.z / 2.0f), false);  // backward
	}

	void draw(DxCore* core, Shader* shader, textureManager textures, sampler sam, mathLib::Matrix& vp) {
		for (auto& c : cubes) {
			c.draw(core, shader, textures, sam, c.worldMatrix, vp);
		}
	}

private:
	void generateWall(DxCore* core, mathLib::Vec3 startPos, bool verticalWall) {
		int numCubes = static_cast<int>(verticalWall ? poolSize.z : poolSize.x);

		for (int i = 0; i < numCubes; ++i) {
			cube c;
			c.init(core);

			mathLib::Vec3 offset = verticalWall
				? mathLib::Vec3(0.0f, 0.0f, i * cubeSize)  // vertical
				: mathLib::Vec3(i * cubeSize, 0.0f, 0.0f); // horizontal

			mathLib::Matrix translation = mathLib::Matrix::translation(startPos + offset);
			c.updateBoundingBox(translation);
			c.worldMatrix = translation;
			cubes.push_back(c);
		}
	}

};

class sphere {
public:
	Mesh mesh;

	void init(DxCore* core, int rings, int segments, float radius) {
		std::vector<STATIC_VERTEX> vertices;
		for (int lat = 0; lat <= rings; lat++) {
			float theta = lat * M_PI / rings;
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);
			for (int lon = 0; lon <= segments; lon++) {
				float phi = lon * 2.0f * M_PI / segments;
				float sinPhi = sinf(phi);
				float cosPhi = cosf(phi);
				mathLib::Vec3 position(radius * sinTheta * cosPhi, radius * cosTheta, radius * sinTheta * sinPhi);
				mathLib::Vec3 normal = position.normalize();
				float tu = 1.0f - (float)lon / segments;
				float tv = 1.0f - (float)lat / rings;
				vertices.push_back(addVertex(position, normal, tu, tv));
			}
		}

		std::vector<unsigned int> indices;
		for (int lat = 0; lat < rings; lat++)
		{
			for (int lon = 0; lon < segments; lon++)
			{
				int current = lat * (segments + 1) + lon;
				int next = current + segments + 1;
				indices.push_back(current);
				indices.push_back(next);
				indices.push_back(current + 1);
				indices.push_back(current + 1);
				indices.push_back(next);
				indices.push_back(next + 1);
			}
		}

		mesh.init(core, vertices, indices);
	}

	void draw(DxCore* core) {
		mesh.draw(core);
	}

	STATIC_VERTEX addVertex(mathLib::Vec3 p, mathLib::Vec3 n, float tu, float tv)
	{
		STATIC_VERTEX v;
		v.pos = p;
		v.normal = n;
		//Frame frame;
		//frame.fromVector(n);
		//v.tangent = frame.u; // For now
		v.tangent = mathLib::Vec3(0, 0, 0);
		v.tu = tu;
		v.tv = tv;
		return v;
	}
};

class river {
public:
	Mesh mesh;

	void init(DxCore* core) {
		std::vector<STATIC_VERTEX> vertices;
		std::vector<unsigned int> indices;
		int gridSize = 10;
		float planeSize = 3.0f;
		float step = planeSize * 2.0f / gridSize;

		for (int z = 0; z <= gridSize; ++z) {
			for (int x = 0; x <= gridSize; ++x) {
				float posX = -planeSize + x * step;
				float posZ = -planeSize + z * step;
				float u = static_cast<float>(x) / gridSize;
				float v = static_cast<float>(z) / gridSize;
				vertices.push_back(addVertex(mathLib::Vec3(posX, 0.0f, posZ), mathLib::Vec3(0, 1, 0), u, v));
			}
		}

		for (int z = 0; z < gridSize; ++z) {
			for (int x = 0; x < gridSize; ++x) {
				int topLeft = z * (gridSize + 1) + x;
				int topRight = topLeft + 1;
				int bottomLeft = (z + 1) * (gridSize + 1) + x;
				int bottomRight = bottomLeft + 1;

				indices.push_back(topLeft);
				indices.push_back(bottomLeft);
				indices.push_back(topRight);

				indices.push_back(topRight);
				indices.push_back(bottomLeft);
				indices.push_back(bottomRight);
			}
		}
		mesh.init(core, vertices, indices);
	}

	// ask the GPU to draw a plane
	void draw(DxCore* core, Shader* shader, textureManager textures, sampler sam, mathLib::Matrix worldMatrix, mathLib::Matrix vp) {
		shader->updateConstantVS("staticMeshBuffer", "W", &worldMatrix);
		shader->updateConstantVS("staticMeshBuffer", "VP", &vp);
		shader->apply(core);
		shader->updateTexturePS(core, "tex", textures.find("Textures/Water_002_COLOR.png"), sam.state);
		shader->updateTexturePS(core, "normalMap", textures.find("Textures/Water_002_NORM.png"), sam.state);
		mesh.draw(core);
	}

};

class model {
public:
	std::vector<Mesh> meshes;
	std::vector<std::string> textureFilenames;
	std::vector<std::string> textureNormalFilenames;

	void init(std::string filename, DxCore* core) {
		GEMLoader::GEMModelLoader loader;
		std::vector<GEMLoader::GEMMesh> gemmeshes;
		loader.load(filename, gemmeshes);
		for (int i = 0; i < gemmeshes.size(); i++) {
			Mesh mesh;
			std::vector<STATIC_VERTEX> vertices;
			for (int j = 0; j < gemmeshes[i].verticesStatic.size(); j++) {
				STATIC_VERTEX v;
				memcpy(&v, &gemmeshes[i].verticesStatic[j], sizeof(STATIC_VERTEX));
				vertices.push_back(v);
			}

			textureFilenames.push_back(gemmeshes[i].material.find("diffuse").getValue());
			textureNormalFilenames.push_back(gemmeshes[i].material.find("normals").getValue());
			mesh.init(core, vertices, gemmeshes[i].indices);
			meshes.push_back(mesh);
		}
	}

	void draw(DxCore* core, Shader* shader, textureManager textures, sampler sam, mathLib::Matrix worldMatrix, mathLib::Matrix& vp) {
		shader->updateConstantVS("staticMeshBuffer", "W", &worldMatrix);
		shader->updateConstantVS("staticMeshBuffer", "VP", &vp);
		shader->apply(core);
		for (int i = 0; i < meshes.size(); i++)
		{
			shader->updateTexturePS(core, "tex", textures.find(textureFilenames[i]), sam.state);
			shader->updateTexturePS(core, "normalMap", textures.find(textureNormalFilenames[i]), sam.state);
			meshes[i].draw(core);
		}
	}
};

class forest {
public:
	std::vector<mathLib::Matrix> transforms; // Transformation matrix for each tree
	model tree; // single tree

	void init(const std::string& modelFilename, DxCore* dx, int treeCount) {
		tree.init(modelFilename, dx);

		// Randomized Spanning Tree Transformations
		for (int i = 0; i < treeCount; ++i) {
			float x = randFloat(-50.0f, 50.0f); // random x
			float z = randFloat(-50.0f, 50.0f); // random z
			float y = 0.0f;                     // fixed y

			float scale = randFloat(0.01f, 0.03f); // random scaling
			float rotation = randFloat(0.0f, 360.0f); // random rotation

			mathLib::Matrix worldMatrix =
				mathLib::Matrix::scaling(mathLib::Vec3(scale, scale, scale)) *
				mathLib::Matrix::rotateY(rotation) *
				mathLib::Matrix::translation(mathLib::Vec3(x, y, z));

			transforms.push_back(worldMatrix);
		}
	}

	void draw(DxCore* dx, textureManager& textures, Shader* shader, sampler& sam, mathLib::Matrix& vp) {
		for (auto& transform : transforms)
			tree.draw(dx, shader, textures, sam, transform, vp);
	}

private:
	float randFloat(float min, float max) {
		return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
	}
};

class animatedModel {
public:
	std::vector<Mesh> meshes;
	Animation animation;
	AnimationInstance instance;
	std::vector<std::string> textureFilenames;
	std::vector<std::string> textureNormalFilenames;
	AABB bounds;


	void init(std::string filename, DxCore* core) {
		GEMLoader::GEMModelLoader loader;
		std::vector<GEMLoader::GEMMesh> gemmeshes;
		GEMLoader::GEMAnimation gemanimation;
		loader.load(filename, gemmeshes, gemanimation);
		for (int i = 0; i < gemmeshes.size(); i++) {
			Mesh mesh;
			std::vector<ANIMATED_VERTEX> vertices;
			for (int j = 0; j < gemmeshes[i].verticesAnimated.size(); j++) {
				ANIMATED_VERTEX v;
				memcpy(&v, &gemmeshes[i].verticesAnimated[j], sizeof(ANIMATED_VERTEX));
				vertices.push_back(v);
			}

			// Load texture with filename: gemmeshes[i].material.find("diffuse").getValue()
			textureFilenames.push_back(gemmeshes[i].material.find("diffuse").getValue());
			textureNormalFilenames.push_back(gemmeshes[i].material.find("normals").getValue());
			mesh.init(core, vertices, gemmeshes[i].indices);
			meshes.push_back(mesh);
		}

		// init Bones
		for (int i = 0; i < gemanimation.bones.size(); i++)
		{
			Bone bone;
			bone.name = gemanimation.bones[i].name;
			memcpy(&bone.offset, &gemanimation.bones[i].offset, 16 * sizeof(float));
			bone.parentIndex = gemanimation.bones[i].parentIndex;
			animation.skeleton.bones.push_back(bone);
		}

		// animation copy data
		for (int i = 0; i < gemanimation.animations.size(); i++)
		{
			std::string name = gemanimation.animations[i].name;
			AnimationSequence aseq;
			aseq.ticksPerSecond = gemanimation.animations[i].ticksPerSecond;
			for (int n = 0; n < gemanimation.animations[i].frames.size(); n++)
			{
				AnimationFrame frame;
				for (int index = 0; index < gemanimation.animations[i].frames[n].positions.size(); index++)
				{
					mathLib::Vec3 p;
					mathLib::Quaternion q;
					mathLib::Vec3 s;
					memcpy(&p, &gemanimation.animations[i].frames[n].positions[index], sizeof(mathLib::Vec3));
					frame.positions.push_back(p);
					memcpy(&q, &gemanimation.animations[i].frames[n].rotations[index], sizeof(mathLib::Quaternion));
					frame.rotations.push_back(q);
					memcpy(&s, &gemanimation.animations[i].frames[n].scales[index], sizeof(mathLib::Vec3));
					frame.scales.push_back(s);
				}
				aseq.frames.push_back(frame);
			}
			animation.animations.insert({ name, aseq });
		}

		instance.animation = &animation;
		calculateBoundingBox();
	}

	void calculateBoundingBox() {
		if (meshes.empty()) return;

		mathLib::Vec3 minPos(FLT_MAX, FLT_MAX, FLT_MAX);
		mathLib::Vec3 maxPos(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		for (const auto& mesh : meshes) {
			std::vector<ANIMATED_VERTEX> vertices;
			for (const auto& vertex : mesh.animatedVertices) {
				const mathLib::Vec3& pos = vertex.pos;

				if (pos.x < minPos.x) minPos.x = pos.x;
				if (pos.y < minPos.y) minPos.y = pos.y;
				if (pos.z < minPos.z) minPos.z = pos.z;

				if (pos.x > maxPos.x) maxPos.x = pos.x;
				if (pos.y > maxPos.y) maxPos.y = pos.y;
				if (pos.z > maxPos.z) maxPos.z = pos.z;
			}
		}

		bounds.min = minPos;
		bounds.max = maxPos;
	}

	void draw(DxCore* core, Shader* shader, textureManager textures, sampler sam, mathLib::Matrix& worldMatrix, mathLib::Matrix& vp) {
		shader->updateConstantVS("animatedMeshBuffer", "W", &worldMatrix);
		shader->updateConstantVS("animatedMeshBuffer", "VP", &vp);
		shader->updateConstantVS("animatedMeshBuffer", "bones", &(this->instance.matrices));
		shader->apply(core);
		for (int i = 0; i < meshes.size(); i++)
		{
			shader->updateTexturePS(core, "tex", textures.find(textureFilenames[i]), sam.state);
			shader->updateTexturePS(core, "normalMap", textures.find(textureNormalFilenames[i]), sam.state);
			meshes[i].draw(core);
		}

		//core->lightingPassInit();
	}
};

class SkyDome {
private:
	Mesh domeMesh; // mesh
	std::string textureFilename;  // texture of sky dome
	mathLib::Matrix worldMatrix; // worldMatrix of sky dome
	float rotationSpeed;  // rotation speed of sky dome

public:
	void init(DxCore* core, int rings, int segments, float radius, const std::string& textureFile) {
		textureFilename = textureFile;

		// create vertices
		std::vector<STATIC_VERTEX> vertices;
		for (int lat = 0; lat <= rings; ++lat) {
			float theta = lat * M_PI / rings;
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);

			for (int lon = 0; lon <= segments; ++lon) {
				float phi = lon * 2.0f * M_PI / segments;
				float sinPhi = sinf(phi);
				float cosPhi = cosf(phi);

				// compute position and textcoords
				mathLib::Vec3 position = mathLib::Vec3(radius * sinTheta * cosPhi, radius * cosTheta, radius * sinTheta * sinPhi);
				mathLib::Vec3 normal = position.normalize();
				float tu = 1.0f - (float)lon / segments;  // texcoord u
				float tv = (float)lat / rings;     // texcoord v
				vertices.push_back(addVertex(position, normal, tu, tv));
			}
		}

		// create indices
		std::vector<unsigned int> indices;
		for (int lat = 0; lat < rings; ++lat) {
			for (int lon = 0; lon < segments; ++lon) {
				int current = lat * (segments + 1) + lon;
				int next = current + segments + 1;
				indices.push_back(current);
				indices.push_back(next);
				indices.push_back(current + 1);
				indices.push_back(current + 1);
				indices.push_back(next);
				indices.push_back(next + 1);
			}
		}

		rotationSpeed = 0.01f;
		domeMesh.init(core, vertices, indices);
	}

	void update(float deltaTime) {
		// rotate with time
		worldMatrix = worldMatrix.rotateY(rotationSpeed * deltaTime);
	}

	// draw Sky Dome
	void draw(DxCore* core, Shader* shader, textureManager& textures, sampler& sam, const mathLib::Vec3& cameraPosition, mathLib::Matrix& vp) {
		// update to camera center
		mathLib::Matrix translation = mathLib::Matrix::translation(cameraPosition);
		mathLib::Matrix finalMatrix = translation * worldMatrix;

		// update view matrix
		shader->updateConstantVS("staticMeshBuffer", "W", &finalMatrix);
		shader->updateConstantVS("staticMeshBuffer", "VP", &vp);

		// bind textures and apply shader
		shader->apply(core);
		shader->updateTexturePS(core, "tex", textures.find(textureFilename), sam.state);
		domeMesh.draw(core);
	}

	void unload(textureManager& textures) {
		textures.unload(textureFilename);
	}
};