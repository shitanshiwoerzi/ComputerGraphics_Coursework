#pragma once
#include "mathLib.h"
#include <d3d11.h>
#include <dxcore.h>
#include <corecrt_math_defines.h>

struct STATIC_VERTEX
{
	mathLib::Vec3 pos;
	mathLib::Vec3 normal;
	mathLib::Vec3 tangent;
	float tu;
	float tv;
};

struct Vertex
{
	// screen space position and colour
	mathLib::Vec3 position;
	mathLib::Color color;
};

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

	void updateConstantVS() {

	}

	void updateConstant() {

	}
};

class Mesh {
public:
	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* vertexBuffer;
	int indicesSize;
	UINT strides;

	// create a plane
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
		init(core, &vertices[0], sizeof(STATIC_VERTEX), vertices.size(), &indices[0], indices.size());
	}

	// ask the GPU to draw a plane
	void draw(ID3D11DeviceContext* devicecontext) {
		UINT offsets = 0;
		devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		devicecontext->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offsets);
		devicecontext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		devicecontext->DrawIndexed(indicesSize, 0, 0);

	}
};

class plane {
public:
	Mesh mesh;

	void init(DxCore* core) {
		std::vector<STATIC_VERTEX> vertices;
		vertices.push_back(addVertex(mathLib::Vec3(-15, 0, -15), mathLib::Vec3(0, 1, 0), 0, 0));
		vertices.push_back(addVertex(mathLib::Vec3(15, 0, -15), mathLib::Vec3(0, 1, 0), 1, 0));
		vertices.push_back(addVertex(mathLib::Vec3(-15, 0, 15), mathLib::Vec3(0, 1, 0), 0, 1));
		vertices.push_back(addVertex(mathLib::Vec3(15, 0, 15), mathLib::Vec3(0, 1, 0), 1, 1));
		std::vector<unsigned int> indices;
		indices.push_back(2); indices.push_back(1); indices.push_back(0);
		indices.push_back(1); indices.push_back(2); indices.push_back(3);
		mesh.init(core, vertices, indices);

	}

	void draw(DxCore* core) {
		mesh.draw(core->devicecontext);
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

class cube {

};

class sphere {
public:
	std::vector<STATIC_VERTEX> vertices;


	void init(int rings, int segments, float radius) {
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

