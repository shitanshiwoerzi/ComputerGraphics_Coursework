#pragma once
#include "mathLib.h"
#include <d3d11.h>

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
};


