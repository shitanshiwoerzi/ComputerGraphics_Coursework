#include "window.h"
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
	std::string avs = "Resources/Shader/animationvertexShader.hlsl";
	std::string vs = "Resources/Shader/vertexShader.hlsl";
	std::string ps = "Resources/Shader/pixelShader.hlsl";
	std::string tps = "Resources/Shader/texturePixelShader.hlsl";
	std::string shaderName = "MyShader";
	std::string planeShaderName = "planeShader";
	plane pl;
	pl.init(dx);

	animatedModel gun;
	gun.init("Resources/GemModel/Automatic_Carbine_5.gem", dx);

	animatedModel soldier;
	soldier.init("Resources/GemModel/Soldier1.gem", dx);

	animatedModel am;
	am.init("Resources/GemModel/TRex.gem", dx);
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
	auto p = m.perspectiveProjection(1.f, 60.0f * M_PI / 180.0f, 100.f, 0.1f);
	//camera camera(0.0f, 5.0f, -10.0f, 0.0f, 0.0f);
	FPSCamera camera(mathLib::Vec3(0.0f, 1.8f, 5.0f));

	textureManager textures;
	textures.load(dx, "Resources/Textures/T-rex_Base_Color.png");
	textures.load(dx, "Resources/Textures/MaleDuty_3_OBJ_Serious_Packed0_Diffuse.png");
	textures.load(dx, "Resources/Textures/arms_1_Albedo.png");
	textures.load(dx, "Resources/Textures/AC5_Albedo.png");
	textures.load(dx, "Resources/Textures/AC5_Collimator_Albedo.png");
	textures.load(dx, "Resources/Textures/AC5_Collimator_Glass_Albedo.png");
	textures.load(dx, "Resources/Textures/AC5_Bullet_Shell_Albedo.png");
	sampler sam;
	sam.init(dx);

	while (true) {
		dx->clear();
		t += tim.dt();
		mathLib::Vec3 from = mathLib::Vec3(11 * cosf(t), 5, 11 * sinf(t));
		//mathLib::Vec3 from = mathLib::Vec3(0.0f, 5.0f, -10.0f);
		mathLib::Matrix v = lookAt(from, to, up);

		camera.processKeyboard(canvas, tim.dt() * 2000);
		camera.processMouse(canvas);

		mathLib::Matrix cv = camera.getViewMatrix();
		vp = cv * p;
		mathLib::Matrix soldierWorld = planeWorld.scaling(mathLib::Vec3(0.01f, 0.01f, 0.01f)) * planeWorld.translation(mathLib::Vec3(3.f, 0, 0));
		mathLib::Matrix dinosaurWorld = planeWorld.rotateY(180);
		planeShader->updateConstantVS("staticMeshBuffer", "W", &planeWorld);
		planeShader->updateConstantVS("staticMeshBuffer", "VP", &vp);
		planeShader->apply(dx);
		pl.draw(dx);
		//sphere.draw(dx);
		// //cub.draw(dx);


		am.instance.update("Run", tim.dt() * 20);
		shader->updateConstantVS("animatedMeshBuffer", "W", &planeWorld);
		shader->updateConstantVS("animatedMeshBuffer", "VP", &vp);
		shader->updateConstantVS("animatedMeshBuffer", "bones", &(am.instance.matrices));
		shader->apply(dx);
		am.draw(dx, shader, textures, sam);

		soldier.instance.update("walking", tim.dt() * 20);
		shader->updateConstantVS("animatedMeshBuffer", "W", &soldierWorld);
		shader->updateConstantVS("animatedMeshBuffer", "bones", &(soldier.instance.matrices));
		shader->apply(dx);
		soldier.draw(dx, shader, textures, sam);

		mathLib::Matrix gunWorld1 = mathLib::Matrix::rotateX(90);
		mathLib::Matrix gunScale = mathLib::Matrix::scaling(mathLib::Vec3(0.2f, 0.2f, 0.2f));
		mathLib::Matrix gunTranslation = mathLib::Matrix::translation(mathLib::Vec3(0.5f, 15.5f, 0.0f));
		mathLib::Matrix gunWorld = gunTranslation * gunScale * mathLib::Matrix::rotateX(90) * cv.invert();

		//gun.instance.update("Armature|08 Fire", tim.dt());
		//gun.instance.update("Armature|00 Pose", tim.dt());
		gun.instance.update("Armature|04 Idle", tim.dt());
		shader->updateConstantVS("animatedMeshBuffer", "W", &gunWorld);
		shader->updateConstantVS("animatedMeshBuffer", "bones", &(gun.instance.matrices));
		shader->apply(dx);
		gun.draw(dx, shader, textures, sam);
		canvas.processMessages();
		dx->present();
	}

	textures.unload("Resources/Textures/T-rex_Base_Color.png");
	textures.unload("Resources/Textures/MaleDuty_3_OBJ_Happy_Packed0_Diffuse.png");
}