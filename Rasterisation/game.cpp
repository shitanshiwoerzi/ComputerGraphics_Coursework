#include "window.h"
#include "dxCore.h"
#include "shader.h"
#include "mesh.h"
#include "GamesEngineeringBase.h"
#include "camera.h"
#include "texture.h"
#include "shooting.h"

static void renderTree(float dt, model tree, Shader* treeShader, mathLib::Matrix planeWorld, mathLib::Matrix vp, DxCore* dx, textureManager textures, sampler sam) {
	mathLib::Vec3 windDirection(1.0f, 0.0f, 0.0f);
	float windSpeed = 1.0f;
	float windStrength = 0.5f;
	treeShader->updateConstantVS("windBuffer", "windDirection", &windDirection);
	treeShader->updateConstantVS("windBuffer", "windSpeed", &windSpeed);
	treeShader->updateConstantVS("windBuffer", "windStrength", &windStrength);
	treeShader->updateConstantVS("windBuffer", "time", &dt);
	tree.draw(dx, treeShader, textures, sam, planeWorld, vp);
}

static void loadAssets(textureManager& textures, DxCore* dx) {
	textures.load(dx, "Resources/Textures/Textures1.png");
	textures.load(dx, "Resources/Textures/grass_003_Mesh.2387_normals.bmp");
	textures.load(dx, "Resources/Textures/plant02.png");
	textures.load(dx, "Resources/Textures/plant02_Normal.png");
	textures.load(dx, "Resources/Textures/bamboo branch.png");
	textures.load(dx, "Resources/Textures/bamboo branch_Normal.png");
	textures.load(dx, "Resources/Textures/T-rex_Base_Color.png");
	textures.load(dx, "Resources/Textures/T-rex_Normal_OpenGL.png");
	textures.load(dx, "Resources/Textures/MaleDuty_3_OBJ_Serious_Packed0_Diffuse.png");
	textures.load(dx, "Resources/Textures/MaleDuty_3_OBJ_Serious_Packed0_Normal.png");
	textures.load(dx, "Resources/Textures/arms_1_Albedo.png");
	textures.load(dx, "Resources/Textures/arms_1_Normal.png");
	textures.load(dx, "Resources/Textures/AC5_Albedo.png");
	textures.load(dx, "Resources/Textures/AC5_Normal.png");
	textures.load(dx, "Resources/Textures/AC5_Collimator_Albedo.png");
	textures.load(dx, "Resources/Textures/AC5_Collimator_Normal.png");
	textures.load(dx, "Resources/Textures/AC5_Collimator_Glass_Albedo.png");
	textures.load(dx, "Resources/Textures/Automatic_Carbine_5_Collimator_normals.bmp");
	textures.load(dx, "Resources/Textures/AC5_Bullet_Shell_Albedo.png");
	textures.load(dx, "Resources/Textures/AC5_Bullet_Shell_Normal.png");
	textures.load(dx, "Resources/Textures/sunsetSky.png");
	textures.load(dx, "Resources/Textures/grass.png");
	textures.load(dx, "Resources/Textures/grass_normal.png");
}


int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
	Window canvas;
	ShaderManager shaders;
	canvas.Init("MyWindow", 1024, 768);
	DxCore* dx = new DxCore();
	dx->Init(1024, 768, canvas.hwnd);
	std::string avs = "Resources/Shader/animationvertexShader.hlsl";
	std::string vs = "Resources/Shader/vertexShader.hlsl";
	std::string lvs = "Resources/Shader/leafVertexShader.hlsl";
	std::string ps = "Resources/Shader/pixelShader.hlsl";
	std::string tps = "Resources/Shader/texturePixelShader.hlsl";
	std::string normalPS = "Resources/Shader/normalPixelShader.hlsl";
	std::string shaderName = "MyShader";
	std::string planeShaderName = "planeShader";
	std::string skyShaderName = "skyShader";
	std::string treeShaderName = "treeShader";

	textureManager textures;
	loadAssets(textures, dx);

	plane pl;
	pl.init(dx);

	cube cube;
	cube.init(dx);

	forest grasses;
	grasses.init("Resources/GemModel/grass_003.gem", dx, 30);

	forest trees;
	trees.init("Resources/GemModel/bamboo.gem", dx, 30);

	//animatedModel soldier;
	//soldier.init("Resources/GemModel/Soldier1.gem", dx);

	animatedModel trex;
	trex.init("Resources/GemModel/TRex.gem", dx);

	SkyDome sky;
	sky.init(dx, 20, 20, 50.0f, "Textures/sunsetSky.png");

	shaders.load(shaderName, avs, normalPS, dx);
	shaders.load(treeShaderName, lvs, normalPS, dx);
	shaders.load(planeShaderName, vs, tps, dx);
	shaders.load(skyShaderName, vs, tps, dx);
	Shader* shader = shaders.getShader(shaderName);
	Shader* planeShader = shaders.getShader(planeShaderName);
	Shader* skyShader = shaders.getShader(skyShaderName);
	Shader* treeShader = shaders.getShader(treeShaderName);
	GamesEngineeringBase::Timer tim;
	float t = 0;
	mathLib::Matrix planeWorld;
	mathLib::Matrix vp;
	mathLib::Vec3 to(0, 1, 0);
	mathLib::Vec3 up(0, 1, 0);
	mathLib::Matrix m;
	auto p = m.perspectiveProjection(1.f, 60.0f * M_PI / 180.0f, 200.f, 0.1f);
	Player player(mathLib::Vec3(0.0f, 1.0f, 0.0f), 5.0f, &trex);
	TPSCamera camera(&player, 5.0f);

	sampler sam;
	sam.init(dx);

	while (true) {
		dx->clear();
		t += tim.dt();
		handleInput(player, camera, canvas, tim.dt() * 20000, cube.boundingBox);
		//player.handleCollision(cube);
		mathLib::Matrix cv = camera.getViewMatrix();
		vp = cv * p;
		// draw sky dome
		sky.update(t);
		sky.draw(dx, skyShader, textures, sam, camera.position, vp);

		// world Matrix
		mathLib::Matrix cubeWorld = planeWorld.translation(mathLib::Vec3(13.f, 0.f, 0.f));

		cube.updateBoundingBox(cubeWorld);
		cube.draw(dx, planeShader, cubeWorld, vp);
		pl.draw(dx, planeShader, textures, sam, planeWorld, vp);
		grasses.draw(dx, textures, treeShader, sam, vp);
		trees.draw(dx, textures, treeShader, sam, vp);
		player.draw(dx, shader, textures, sam, vp);
		canvas.processMessages();
		dx->present();
	}

	textures.unload("Resources/Textures/T-rex_Base_Color.png");
	textures.unload("Resources/Textures/MaleDuty_3_OBJ_Happy_Packed0_Diffuse.png");
}