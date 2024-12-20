﻿#pragma once
#include <d3d11.h>
#include "adapter.h"

class DxCore {
public:
	ID3D11Device* device;
	ID3D11DeviceContext* devicecontext;
	IDXGISwapChain* swapchain;
	ID3D11RenderTargetView* backbufferRenderTargetView;
	ID3D11Texture2D* backbuffer;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11Texture2D* depthbuffer;
	D3D11_VIEWPORT viewport;
	ID3D11RasterizerState* rasterizerState;

	ID3D11RenderTargetView* gBufferRTV[3];
	ID3D11ShaderResourceView* gBufferSRV[3];

	//ID3D11BlendState* blendState;

	void Init(unsigned int width, unsigned int height, HWND hwnd, bool window_fullscreen = false) {
		DXGI_SWAP_CHAIN_DESC sd;
		memset(&sd, 0, sizeof(DXGI_SWAP_CHAIN_DESC));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hwnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = window_fullscreen ? false : true;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		D3D_FEATURE_LEVEL fl;
		fl = D3D_FEATURE_LEVEL_11_0;

		Adapter a;
		a.findAdapter();
		D3D11CreateDeviceAndSwapChain(a.adapter,
			D3D_DRIVER_TYPE_UNKNOWN,
			NULL,
			D3D11_CREATE_DEVICE_DEBUG,
			&fl,
			1,
			D3D11_SDK_VERSION,
			&sd,
			&swapchain,
			&device,
			NULL,
			&devicecontext);

		swapchain->SetFullscreenState(window_fullscreen, NULL);
		swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);
		device->CreateRenderTargetView(backbuffer, NULL, &backbufferRenderTargetView);

		D3D11_TEXTURE2D_DESC dsvDesc;
		dsvDesc.Width = width;
		dsvDesc.Height = height;
		dsvDesc.MipLevels = 1;
		dsvDesc.ArraySize = 1;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.SampleDesc.Count = 1;
		dsvDesc.SampleDesc.Quality = 0;
		dsvDesc.Usage = D3D11_USAGE_DEFAULT;
		dsvDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		dsvDesc.CPUAccessFlags = 0;
		dsvDesc.MiscFlags = 0;

		device->CreateTexture2D(&dsvDesc, NULL, &depthbuffer);
		device->CreateDepthStencilView(depthbuffer, NULL, &depthStencilView);

		initGBuffer(width, height);
		devicecontext->OMSetRenderTargets(1, &backbufferRenderTargetView, depthStencilView);

		viewport.Width = (float)width;
		viewport.Height = (float)height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		devicecontext->RSSetViewports(1, &viewport);

		rasterizerConfig();
	}

	void geometryPass() {
		// set G Buffer as default
		ID3D11RenderTargetView* rtvs[3] = { gBufferRTV[0], gBufferRTV[1], gBufferRTV[2] };
		devicecontext->OMSetRenderTargets(3, rtvs, depthStencilView);

		// clear G buffer
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		for (int i = 0; i < 3; ++i) {
			devicecontext->ClearRenderTargetView(gBufferRTV[i], clearColor);
		}
		// clear the depth buffer again
		devicecontext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	void lightingPass() {
		// unbundle
		ID3D11RenderTargetView* nullRTV[3] = { nullptr, nullptr, nullptr };
		devicecontext->OMSetRenderTargets(3, nullRTV, nullptr);

		// set back buffer to default
		devicecontext->OMSetRenderTargets(1, &backbufferRenderTargetView, nullptr);

		devicecontext->PSSetShaderResources(0, 3, gBufferSRV);

		devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		devicecontext->Draw(3, 0);
	}

	void clear() {
		float ClearColour[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
		devicecontext->ClearRenderTargetView(backbufferRenderTargetView, ClearColour);
		devicecontext->ClearDepthStencilView(depthStencilView,
			D3D11_CLEAR_DEPTH |
			D3D11_CLEAR_STENCIL,
			1.0f,
			0);
	}

	void present() {
		swapchain->Present(0, 0);
	}

private:
	// configure the rasterizer
	void rasterizerConfig() {
		D3D11_RASTERIZER_DESC rsdesc;
		ZeroMemory(&rsdesc, sizeof(D3D11_RASTERIZER_DESC));
		rsdesc.FillMode = D3D11_FILL_SOLID;
		rsdesc.CullMode = D3D11_CULL_NONE;
		device->CreateRasterizerState(&rsdesc, &rasterizerState);
		devicecontext->RSSetState(rasterizerState);
	}

	void initGBuffer(unsigned int width, unsigned int height) {
		DXGI_FORMAT formats[3] = {
					DXGI_FORMAT_R32G32B32A32_FLOAT, // Position
					DXGI_FORMAT_R32G32B32A32_FLOAT, // Normal
					DXGI_FORMAT_R8G8B8A8_UNORM      // Albedo + Specular
		};

		for (int i = 0; i < 3; ++i) {
			D3D11_TEXTURE2D_DESC texDesc = {};
			texDesc.Width = width;
			texDesc.Height = height;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.Format = formats[i];
			texDesc.SampleDesc.Count = 1;
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

			ID3D11Texture2D* texture = nullptr;
			device->CreateTexture2D(&texDesc, nullptr, &texture);
			device->CreateRenderTargetView(texture, nullptr, &gBufferRTV[i]);
			device->CreateShaderResourceView(texture, nullptr, &gBufferSRV[i]);
			texture->Release();
		}
	}

	//void blend() {
	//	device->CreateBlendState(&blendDesc, &blendState);
	//	devicecontext->OMSetBlendState(blendState, NULL, 0xffffffff);
	//}
};