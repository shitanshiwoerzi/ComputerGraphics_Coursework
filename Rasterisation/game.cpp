#include "window.h"
#include "dxCore.h"
#include "shader.h"
#include "mesh.h"
#include "GamesEngineeringBase.h"
#include "camera.h"
#include "texture.h"
#include "shooting.h"

static void renderWater(float dt, river water, Shader* waterShader, mathLib::Matrix planeWorld, mathLib::Matrix vp, DxCore* core, textureManager textures, sampler sam) {
	float waveFrequency = 1.f;
	float waveSpeed = 1.0f;
	float waveAmplitude = 0.5f;
	waterShader->updateConstantVS("waterParams", "waveSpeed", &waveSpeed);
	waterShader->updateConstantVS("waterParams", "waveAmplitude", &waveAmplitude);
	waterShader->updateConstantVS("waterParams", "waveFrequency", &waveFrequency);
	waterShader->updateConstantVS("waterParams", "time", &dt);
	water.draw(core, waterShader, textures, sam, planeWorld, vp);
}

static void loadAssets(textureManager& textures, DxCore* core) {
	textures.load(core, "Resources/Textures/Textures1.png");
	textures.load(core, "Resources/Textures/grass_003_Mesh.2387_normals.bmp");
	textures.load(core, "Resources/Textures/plant02.png");
	textures.load(core, "Resources/Textures/plant02_Normal.png");
	textures.load(core, "Resources/Textures/bamboo branch.png");
	textures.load(core, "Resources/Textures/bamboo branch_Normal.png");
	textures.load(core, "Resources/Textures/T-rex_Base_Color.png");
	textures.load(core, "Resources/Textures/T-rex_Normal_OpenGL.png");
	textures.load(core, "Resources/Textures/MaleDuty_3_OBJ_Serious_Packed0_Diffuse.png");
	textures.load(core, "Resources/Textures/MaleDuty_3_OBJ_Serious_Packed0_Normal.png");
	textures.load(core, "Resources/Textures/arms_1_Albedo.png");
	textures.load(core, "Resources/Textures/arms_1_Normal.png");
	textures.load(core, "Resources/Textures/AC5_Albedo.png");
	textures.load(core, "Resources/Textures/AC5_Normal.png");
	textures.load(core, "Resources/Textures/AC5_Collimator_Albedo.png");
	textures.load(core, "Resources/Textures/AC5_Collimator_Normal.png");
	textures.load(core, "Resources/Textures/AC5_Collimator_Glass_Albedo.png");
	textures.load(core, "Resources/Textures/Automatic_Carbine_5_Collimator_normals.bmp");
	textures.load(core, "Resources/Textures/AC5_Bullet_Shell_Albedo.png");
	textures.load(core, "Resources/Textures/AC5_Bullet_Shell_Normal.png");
	textures.load(core, "Resources/Textures/sunsetSky.png");
	textures.load(core, "Resources/Textures/grass.png");
	textures.load(core, "Resources/Textures/grass_Normal.png");
	textures.load(core, "Resources/Textures/Water_002_COLOR.png");
	textures.load(core, "Resources/Textures/Water_002_NORM.png");
	textures.load(core, "Resources/Textures/Bricks097_1K-PNG_Color.png");
	textures.load(core, "Resources/Textures/Bricks097_1K-PNG_NormalDX.png");
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
	Window canvas;
	ShaderManager shaders;
	canvas.Init("MyWindow", 1024, 768);
	DxCore* dx = new DxCore();
	dx->Init(1024, 768, canvas.hwnd);
	std::string avs = "Resources/Shader/animationvertexShader.hlsl";
	std::string vs = "Resources/Shader/vertexShader.hlsl";
	std::string waterVS = "Resources/Shader/waterVertexShader.hlsl";
	std::string ps = "Resources/Shader/pixelShader.hlsl";
	std::string tps = "Resources/Shader/texturePixelShader.hlsl";
	std::string normalPS = "Resources/Shader/normalPixelShader.hlsl";
	std::string testPS = "Resources/Shader/test.hlsl";
	std::string lightPS = "Resources/Shader/simpleLightPS.hlsl";
	std::string shaderName = "MyShader";
	std::string planeShaderName = "planeShader";
	std::string skyShaderName = "skyShader";
	std::string treeShaderName = "treeShader";
	std::string waterShaderName = "waterShader";

	// defer shading - light shader
	//Shader* lightShader = new Shader();
	//lightShader->loadPS(lightPS, dx);
	//lightShader->Init(dx->device);
	textureManager textures;
	loadAssets(textures, dx);

	plane pl;
	pl.init(dx);

	river water;
	water.init(dx);

	cube cube;
	cube.init(dx);

	Pool pool;
	pool.init(dx, mathLib::Vec3(5, 0, 5), 1);

	forest grasses;
	grasses.init("Resources/GemModel/grass_003.gem", dx, 30);

	forest trees;
	trees.init("Resources/GemModel/bamboo.gem", dx, 30);

	animatedModel trex;
	trex.init("Resources/GemModel/TRex.gem", dx);

	SkyDome sky;
	sky.init(dx, 20, 20, 50.0f, "Textures/sunsetSky.png");

	shaders.load(shaderName, avs, normalPS, dx);
	shaders.load(treeShaderName, vs, normalPS, dx);
	shaders.load(planeShaderName, vs, normalPS, dx);
	shaders.load(skyShaderName, vs, tps, dx);
	shaders.load(waterShaderName, waterVS, normalPS, dx);
	Shader* shader = shaders.getShader(shaderName);
	Shader* planeShader = shaders.getShader(planeShaderName);
	Shader* skyShader = shaders.getShader(skyShaderName);
	Shader* treeShader = shaders.getShader(treeShaderName);
	Shader* waterShader = shaders.getShader(waterShaderName);
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
		// defer shading	
		//dx->geometryPass();
		t += tim.dt();
		handleInput(player, camera, canvas, tim.dt() * 20000, cube.boundingBox);
		mathLib::Matrix cv = camera.getViewMatrix();
		vp = cv * p;
		// draw sky dome
		sky.update(t);
		sky.draw(dx, skyShader, textures, sam, camera.position, vp);

		// world Matrix
		mathLib::Matrix cubeWorld = mathLib::Matrix::translation(mathLib::Vec3(13.f, 1.f, 0.f));
		mathLib::Matrix waterWorld = mathLib::Matrix::translation(mathLib::Vec3(0.f, 1.f, 0.f));

		cube.updateBoundingBox(cubeWorld);
		cube.draw(dx, planeShader, textures, sam, cubeWorld, vp);
		pl.draw(dx, planeShader, textures, sam, planeWorld, vp);
		grasses.draw(dx, textures, treeShader, sam, vp);
		trees.draw(dx, textures, treeShader, sam, vp);
		player.draw(dx, shader, textures, sam, vp);
		pool.draw(dx, planeShader, textures, sam, vp);
		renderWater(t, water, waterShader, waterWorld, vp, dx, textures, sam);

		// defer Shading implementation
		//lightShader->applyPS(dx);
		//dx->devicecontext->PSSetSamplers(0, 1, &sam.state);
		//dx->lightingPass();
		canvas.processMessages();
		dx->present();
	}

	textures.unload("Resources/Textures/T-rex_Base_Color.png");
	textures.unload("Resources/Textures/MaleDuty_3_OBJ_Happy_Packed0_Diffuse.png");
}