﻿#include "window.h"
#include "dxCore.h"
#include "shader.h"
#include "mesh.h"
#include "GamesEngineeringBase.h"
#include "camera.h"
#include "texture.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
	Window canvas;
	ShaderManager shaders;
	canvas.Init("MyWindow", 1024, 768);
	DxCore* dx = new DxCore();
	dx->Init(1024, 768, canvas.hwnd);
	std::string avs = "Resources/animationvertexShader.hlsl";
	std::string vs = "Resources/vertexShader.hlsl";
	std::string ps = "Resources/pixelShader.hlsl";
	std::string tps = "Resources/texturePixelShader.hlsl";
	std::string shaderName = "MyShader";
	std::string planeShaderName = "planeShader";
	plane pl;
	pl.init(dx);

	cube cub;
	cub.init(dx);

	sphere sphere;
	sphere.init(dx, 100, 100, 0.5f);

	model tree;
	tree.init("Resources/acacia_003.gem", dx);

	animatedModel am;
	am.init("Resources/TRex.gem", dx);
	shaders.load(shaderName, avs, tps, dx);
	shaders.load(planeShaderName, vs, ps, dx);
	Shader* shader = shaders.getShader(shaderName);
	Shader* planeShader = shaders.getShader(planeShaderName);
	GamesEngineeringBase::Timer tim;
	float t = 0;
	mathLib::Matrix planeWorld;
	mathLib::Matrix vp;
	mathLib::Vec3 to(0, 1, 0);
	mathLib::Vec3 up(0, 1, 0);
	mathLib::Matrix m;
	auto p = m.perspectiveProjection(1.f, 45.0f * M_PI / 180.0f, 100.f, 0.1f);
	camera camera(0.0f, 5.0f, -10.0f, 0.0f, 0.0f);

	textureManager textures;
	textures.load(dx, "Resources/Textures/T-rex_Base_Color.png");
	sampler sam;
	sam.init(dx);

	while (true) {
		dx->clear();
		t += tim.dt();
		mathLib::Vec3 from = mathLib::Vec3(11 * cosf(t), 5, 11 * sinf(t));
		//mathLib::Vec3 from = mathLib::Vec3(0.0f, 5.0f, -10.0f);
		mathLib::Matrix v = lookAt(from, to, up);

		mathLib::Matrix cv = camera.getViewMatrix();
		camera.processKeyboard(canvas.keys, tim.dt()*2000);
		//camera.processMouseMovement(canvas.mousex, canvas.mousey);

		vp = cv * p;
		mathLib::Matrix treeWorld = planeWorld.scaling(mathLib::Vec3(0.001f, 0.001f, 0.001f));
		planeShader->updateConstantVS("staticMeshBuffer", "W", &planeWorld);
		planeShader->updateConstantVS("staticMeshBuffer", "VP", &vp);

		am.instance.update("Run", tim.dt() * 20);
		shader->updateConstantVS("animatedMeshBuffer", "W", &planeWorld);
		shader->updateConstantVS("animatedMeshBuffer", "VP", &vp);
		shader->updateConstantVS("animatedMeshBuffer", "bones", &(am.instance.matrices));
		planeShader->apply(dx);
		pl.draw(dx);
		//sphere.draw(dx);
		//cub.draw(dx);
		//tree.draw(dx);
		shader->apply(dx);
		am.draw(dx, shader, textures, sam);
		canvas.processMessages();
		dx->present();
	}

	textures.unload("Resources/Textures/T-rex_Base_Color.png");
}