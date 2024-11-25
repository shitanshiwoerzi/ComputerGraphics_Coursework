#include "window.h"
#include "dxCore.h"
#include "shader.h"
#include "mesh.h"
#include "GamesEngineeringBase.h"

struct alignas(16) timeAndLight {
	float time;
	float padding[3];
	mathLib::Vec4 lights[4];
};

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
	Window canvas;
	DxCore* dx = new DxCore();
	//Shader shader;
	ShaderManager shaders;
	Triangle t;
	GamesEngineeringBase::Timer tim;
	float dt;
	timeAndLight* entity = new timeAndLight();
	entity->time = 0;
	t.Init();
	canvas.Init("MyWindow", 1024, 768);
	dx->Init(1024, 768, canvas.hwnd);
	std::string s = "Resources/vertex_shader.txt";
	std::string shaderName = "MyShader";
	shaders.load(shaderName, s, s, dx);
	Shader* shader = shaders.getShader(shaderName);
	t.createBuffer(dx->device);
	while (true) {
		dx->clear();
		dt = tim.dt();
		entity->time += dt;
		for (int i = 0; i < 4; i++)
		{
			float angle = entity->time + (i * M_PI / 2.0f);
			entity->lights[i] = mathLib::Vec4(canvas.width / 2.0f + (cosf(angle) * (canvas.width * 0.3f)),
				canvas.height / 2.0f + (sinf(angle) * (canvas.height * 0.3f)),
				0, 0);
		}

		shader->updateConstantPS("bufferName", "time", &entity->time);
		shader->updateConstantPS("bufferName", "lights", &entity->lights);
		canvas.processMessages();
		shader->apply(dx);
		t.draw(dx->devicecontext);
		dx->present();
	}
}