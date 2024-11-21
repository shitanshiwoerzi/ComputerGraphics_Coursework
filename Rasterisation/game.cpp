#include "window.h"
#include "dxCore.h"
#include "shader.h"
#include "mesh.h"
#include "GamesEngineeringBase.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
	Window canvas;
	DxCore dx;
	Shader shader;
	Triangle t;
	GamesEngineeringBase::Timer tim;
	float dt;
	ConstantBuffer* constBufferCPU = new ConstantBuffer();
	constBufferCPU->time = 0;
	t.Init();
	canvas.Init("MyWindow", 1024, 768);
	dx.Init(1024, 768, canvas.hwnd);
	std::string s = "Resources/vertex_shader.txt";
	ID3DBlob* shaderBlob = nullptr;
	shader.loadVS(s, dx.device);
	shader.loadPS(s, dx.device, &shaderBlob);
	shader.Init(dx.device, shaderBlob);
	shaderBlob->Release();
	t.createBuffer(dx.device);
	while (true) {
		dx.clear();
		dt = tim.dt();
		constBufferCPU->time += dt;
		for (int i = 0; i < 4; i++)
		{
			float angle = constBufferCPU->time + (i * M_PI / 2.0f);
			constBufferCPU->lights[i] = mathLib::Vec4(canvas.width / 2.0f + (cosf(angle) * (canvas.width * 0.3f)),
				canvas.height / 2.0f + (sinf(angle) * (canvas.height * 0.3f)),
				0, 0);
		}

		canvas.processMessages();
		// 更新常量缓冲区中的变量
		shader.update("time", &constBufferCPU->time);
		shader.update("lights", constBufferCPU->lights);

		// 将更新应用到 GPU
		shader.UpdateConstantBuffer(dx.devicecontext);
		//shader.map(constBufferCPU, dx.devicecontext);
		shader.apply(dx.devicecontext);
		t.draw(dx.devicecontext);
		dx.present();
	}
}