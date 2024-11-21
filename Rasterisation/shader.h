#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <string>
#include <fstream>
#include <sstream>
#include <map>

class ShaderManager {
public:
	//std::map<std::string, Shader> shaders;

	void load() {

	}
};

class Shader {
public:
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* layout;

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
			{ "COLOUR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		device->CreateInputLayout(layoutDesc, 2, shader->GetBufferPointer(), shader->GetBufferSize(), &layout);
		shader->Release();
	}

	void loadPS(std::string& filename, ID3D11Device* device) {
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
		device->CreatePixelShader(shader->GetBufferPointer(), shader->GetBufferSize(), NULL, &pixelShader);
		shader->Release();
	}

	void apply(ID3D11DeviceContext* devicecontext) const {
		devicecontext->IASetInputLayout(layout);
		devicecontext->VSSetShader(vertexShader, NULL, 0);
		devicecontext->PSSetShader(pixelShader, NULL, 0);
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