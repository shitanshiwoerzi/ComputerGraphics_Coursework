#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include "mathLib.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

struct ConstantBufferVariable
{
	std::string name;
	unsigned int offset;
	unsigned int size;
};

struct ConstantBufferReflection
{
	std::string name;
	std::map<std::string, ConstantBufferVariable> constantBufferData;
};

struct alignas(16) ConstantBuffer
{
	float time;
	float padding[3];
	mathLib::Vec4 lights[4];
};

class Shader {
public:
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* layout;
	ID3D11Buffer* constantBuffer;
	ConstantBufferReflection cbReflection;
	unsigned char* CPUbuffer;
	int totalSizeInBytes16;
	bool dirty;

	void Init(ID3D11Device* device, ID3DBlob* shaderBlob, int sizeInBytes = 16) {
		D3D11_BUFFER_DESC bd;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA data;
		bd.ByteWidth = sizeInBytes;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		device->CreateBuffer(&bd, NULL, &constantBuffer);

		int totalSizeInBytes = 0;

		// 使用反射系统填充元数据
		ID3D11ShaderReflection* reflection;
		D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection);
		D3D11_SHADER_DESC desc;
		reflection->GetDesc(&desc);
		for (int i = 0; i < desc.ConstantBuffers; i++) {
			ID3D11ShaderReflectionConstantBuffer* constantBuffer = reflection->GetConstantBufferByIndex(i);
			D3D11_SHADER_BUFFER_DESC cbDesc;
			constantBuffer->GetDesc(&cbDesc);
			cbReflection.name = cbDesc.Name;
			for (int j = 0; j < cbDesc.Variables; j++) {
				ID3D11ShaderReflectionVariable* var = constantBuffer->GetVariableByIndex(j);
				D3D11_SHADER_VARIABLE_DESC vDesc;
				var->GetDesc(&vDesc);
				ConstantBufferVariable bufferVariable;
				bufferVariable.offset = vDesc.StartOffset;
				bufferVariable.size = vDesc.Size;
				cbReflection.constantBufferData.insert({ vDesc.Name, bufferVariable });
				totalSizeInBytes += bufferVariable.size;
				totalSizeInBytes16 = ((totalSizeInBytes + 15) & -16);
			}
		}

		// 分配 CPU 缓冲区
		CPUbuffer = new unsigned char[totalSizeInBytes16];
		dirty = false;
		reflection->Release();
	}

	void map(ConstantBuffer* buffer, ID3D11DeviceContext* devicecontext) {
		D3D11_MAPPED_SUBRESOURCE mapped;
		devicecontext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		memcpy(mapped.pData, buffer, sizeof(ConstantBuffer));
		devicecontext->Unmap(constantBuffer, 0);
	}

	void UpdateConstantBuffer(ID3D11DeviceContext* deviceContext) {
		if (dirty) {
			D3D11_MAPPED_SUBRESOURCE mapped;
			deviceContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			memcpy(mapped.pData, CPUbuffer, totalSizeInBytes16);
			deviceContext->Unmap(constantBuffer, 0);
			dirty = false;
		}
	}

	void update(std::string name, void* data) {
		auto it = cbReflection.constantBufferData.find(name);
		if (it != cbReflection.constantBufferData.end()) {
			const ConstantBufferVariable& cbVariable = it->second;
			memcpy(&CPUbuffer[cbVariable.offset], data, cbVariable.size);
			dirty = true;
		}
	}

	void loadVS(std::string& filename, ID3D11Device* device) {
		ID3DBlob* status;
		ID3DBlob* shader;
		std::string shaderHLSL = readFile(filename);
		// compile vertex shader
		HRESULT hr = D3DCompile(shaderHLSL.c_str(), strlen(shaderHLSL.c_str()), NULL, NULL, NULL, "VS", "vs_5_0", 0, 0, &shader, &status);
		if (FAILED(hr)) {
			MessageBoxA(NULL, (char*)status->GetBufferPointer(), "Vertex Shader Error", 0);
			exit(0);
		}
		// create vertex shader
		device->CreateVertexShader(shader->GetBufferPointer(), shader->GetBufferSize(), NULL, &vertexShader);
		D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
		{
			{ "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		device->CreateInputLayout(layoutDesc, 4, shader->GetBufferPointer(), shader->GetBufferSize(), &layout);
		shader->Release();
	}

	void loadPS(std::string& filename, ID3D11Device* device, ID3DBlob** shader) {
		ID3DBlob* status;
		std::string shaderHLSL = readFile(filename);
		// compile pixel shader
		HRESULT hr = D3DCompile(shaderHLSL.c_str(), strlen(shaderHLSL.c_str()), NULL, NULL, NULL, "PS", "ps_5_0", 0, 0, &(*shader), &status);
		if (FAILED(hr)) {
			MessageBoxA(NULL, (char*)status->GetBufferPointer(), "Pixel Shader Error", 0);
			exit(0);
		}
		// create pixel shader
		device->CreatePixelShader((*shader)->GetBufferPointer(), (*shader)->GetBufferSize(), NULL, &pixelShader);
	}

	void apply(ID3D11DeviceContext* devicecontext) const {
		devicecontext->IASetInputLayout(layout);
		devicecontext->VSSetShader(vertexShader, NULL, 0);
		devicecontext->PSSetShader(pixelShader, NULL, 0);
		devicecontext->PSSetConstantBuffers(0, 1, &constantBuffer);
	}

private:
	std::string readFile(std::string& filename) {
		std::ifstream infile;
		infile.open(filename);

		// 使用 stringstream 读取文件内容
		std::stringstream buffer;
		buffer << infile.rdbuf();

		// 返回文件内容作为字符串
		return buffer.str();
	}
};

class ShaderManager {
public:
	std::map<std::string, Shader> shaders;

	void load(std::string& name, std::string& vsFilename, std::string& psFilename, ID3D11Device* device) {
		Shader shader;
		ID3DBlob* shaderBlob = nullptr;
		shader.loadVS(vsFilename, device);
		shader.loadPS(psFilename, device, &shaderBlob);
		shader.Init(device, shaderBlob);
		shaderBlob->Release();
		shaders[name] = shader;
	}

	Shader* getShader(std::string& name) {
		auto it = shaders.find(name);
		if (it != shaders.end()) {
			return &it->second;
		}
		return nullptr;
	}

	void apply(std::string& name, ID3D11DeviceContext* deviceContext) {
		Shader* shader = getShader(name);
		if (shader)
			shader->apply(deviceContext);
	}

};