#include "window.h"
#include "dxCore.h"
#include "shader.h"
#include "mesh.h"
#include "GamesEngineeringBase.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
	Window canvas;
	ShaderManager shaders;
	canvas.Init("MyWindow", 1024, 768);
	DxCore* dx = new DxCore();
	dx->Init(1024, 768, canvas.hwnd);
	std::string vs = "Resources/vertexShader.hlsl";
	std::string ps = "Resources/pixelShader.hlsl";
	std::string shaderName = "MyShader";
	plane pl;
	pl.init(dx);

	cube cub;
	cub.init(dx);

	sphere sphere;
	sphere.init(dx, 100, 100, 0.5f);

	model tree;
	tree.init("Resources/acacia.gem", dx);

	shaders.load(shaderName, vs, ps, dx);
	Shader* shader = shaders.getShader(shaderName);
	GamesEngineeringBase::Timer tim;
	float t = 0;
	mathLib::Matrix planeWorld;
	mathLib::Matrix vp;
	mathLib::Vec3 to(0, 1, 0);
	mathLib::Vec3 up(0, 1, 0);
	mathLib::Matrix m;
	auto p = m.perspectiveProjection(1.f, 45.0f * M_PI / 180.0f, 100.f, 0.1f);

	while (true) {
		dx->clear();
		t += tim.dt();
		mathLib::Vec3 from = mathLib::Vec3(11 * cosf(t), 5, 11 * sinf(t));
		//mathLib::Vec3 from = mathLib::Vec3(0.0f, 5.0f, -10.0f);
		mathLib::Matrix v = lookAt(from, to, up);

		vp = v * p;
		mathLib::Matrix treeWorld = planeWorld.scaling(mathLib::Vec3(0.001f, 0.001f, 0.001f));
		shader->updateConstantVS("staticMeshBuffer", "W", &treeWorld);
		shader->updateConstantVS("staticMeshBuffer", "VP", &vp);
		shader->apply(dx);
		//pl.draw(dx);
		//sphere.draw(dx);
		//cub.draw(dx);
		//tree.draw(dx);
		canvas.processMessages();
		dx->present();
	}
}