#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include "mathLib.h"
#include "shaderReflection.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

class Shader {
public:
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* layout;
	ID3D11Buffer* constantBuffer;
	std::vector<ConstantBuffer> psConstantBuffers;
	std::vector<ConstantBuffer> vsConstantBuffers;
	std::map<std::string, int> textureBindPointsVS;
	std::map<std::string, int> textureBindPointsPS;

	void Init(ID3D11Device* device, int sizeInBytes = 16) {
		D3D11_BUFFER_DESC bd;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA data;
		bd.ByteWidth = sizeInBytes;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		device->CreateBuffer(&bd, NULL, &constantBuffer);
	}

	void loadVS(std::string& filename, DxCore* core) {
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
		core->device->CreateVertexShader(shader->GetBufferPointer(), shader->GetBufferSize(), NULL, &vertexShader);
		ConstantBufferReflection reflection;
		reflection.build(core, shader, vsConstantBuffers, textureBindPointsVS, ShaderStage::VertexShader);
		//D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
		//{
		//	{ "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//};

		D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
			{ "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONEIDS", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		core->device->CreateInputLayout(layoutDesc, 6, shader->GetBufferPointer(), shader->GetBufferSize(), &layout);
		shader->Release();
	}

	void loadPS(std::string& filename, DxCore* core) {
		ID3DBlob* status;
		ID3DBlob* shader;
		std::string shaderHLSL = readFile(filename);
		// compile pixel shader
		HRESULT hr = D3DCompile(shaderHLSL.c_str(), strlen(shaderHLSL.c_str()), NULL, NULL, NULL, "PS", "ps_5_0", 0, 0, &shader, &status);
		if (FAILED(hr)) {
			MessageBoxA(NULL, (char*)status->GetBufferPointer(), "Pixel Shader Error", 0);
			exit(0);
		}
		// create pixel shader
		core->device->CreatePixelShader(shader->GetBufferPointer(), shader->GetBufferSize(), NULL, &pixelShader);
		ConstantBufferReflection reflection;
		reflection.build(core, shader, psConstantBuffers, textureBindPointsPS, ShaderStage::PixelShader);
		shader->Release();
	}

	//void updateConstantVS(std::string name, std::string constBuffferName, std::string variableName, void* data) {
	//	for (int i = 0; i < vsConstantBuffers.size(); i++)
	//	{
	//		if (vsConstantBuffers[i].name == name)
	//		{
	//			vsConstantBuffers[i].update(variableName, data);
	//		}
	//	}
	//}

	//void updateConstantPS(std::string name, std::string constBuffferName, std::string variableName, void* data) {
	//	for (int i = 0; i < psConstantBuffers.size(); i++)
	//	{
	//		if (psConstantBuffers[i].name == name)
	//		{
	//			psConstantBuffers[i].update(variableName, data);
	//		}
	//	}
	//}

	void updateConstantVS(std::string constantBufferName, std::string variableName, void* data)
	{
		updateConstant(constantBufferName, variableName, data, vsConstantBuffers);
	}
	void updateConstantPS(std::string constantBufferName, std::string variableName, void* data)
	{
		updateConstant(constantBufferName, variableName, data, psConstantBuffers);
	}

	void updateConstant(std::string constantBufferName, std::string variableName, void* data, std::vector<ConstantBuffer>& buffers)
	{
		for (int i = 0; i < buffers.size(); i++)
		{
			if (buffers[i].name == constantBufferName)
			{
				buffers[i].update(variableName, data);
				return;
			}
		}
	}

	void updateTexturePS(DxCore* core, std::string name, ID3D11ShaderResourceView* srv, ID3D11SamplerState* state) {
		//core->devicecontext->VSSetShaderResources(textureBindPointsVS[name], 1, &srv);
		core->devicecontext->PSSetShaderResources(textureBindPointsPS[name], 1, &srv);
		core->devicecontext->PSSetSamplers(textureBindPointsPS[name], 1, &state);
	}

	void apply(DxCore* core) {
		core->devicecontext->IASetInputLayout(layout);
		core->devicecontext->VSSetShader(vertexShader, NULL, 0);
		core->devicecontext->PSSetShader(pixelShader, NULL, 0);


		for (int i = 0; i < vsConstantBuffers.size(); i++) {
			vsConstantBuffers[i].upload(core);
		}
		for (int i = 0; i < psConstantBuffers.size(); i++) {
			psConstantBuffers[i].upload(core);
		}
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

	void load(std::string& name, std::string& vsFilename, std::string& psFilename, DxCore* core) {
		Shader shader;
		shader.loadVS(vsFilename, core);
		shader.loadPS(psFilename, core);
		shader.Init(core->device);
		shaders[name] = shader;
	}

	Shader* getShader(std::string& name) {
		auto it = shaders.find(name);
		if (it != shaders.end()) {
			return &it->second;
		}
		return nullptr;
	}

	void apply(std::string& name, DxCore* core) {
		Shader* shader = getShader(name);
		if (shader)
			shader->apply(core);
	}

};