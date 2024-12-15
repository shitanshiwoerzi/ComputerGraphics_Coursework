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

static void renderLight(DxCore* core, Shader* lightShader, mathLib::Vec3 cameraPostion) {
	mathLib::Vec3 lightPos[4];
	mathLib::Vec3 lightColor[4];
	lightPos[0] = mathLib::Vec3(5.0f, 5.0f, 5.0f);
	lightPos[1] = mathLib::Vec3(5.0f, 5.0f, 5.0f);
	lightPos[2] = mathLib::Vec3(-5.0f, 5.0f, -5.0f);
	lightPos[3] = mathLib::Vec3(5.0f, 10.0f, -5.0f);
	lightColor[0] = mathLib::Vec3(1.0f, 1.0f, 1.0f);
	lightColor[1] = mathLib::Vec3(1.0f, 0.0f, 0.0f);
	lightColor[2] = mathLib::Vec3(0.0f, 1.0f, 0.0f);
	lightColor[3] = mathLib::Vec3(0.0f, 0.0f, 1.0f);
	int numLight = 1;
	lightShader->updateConstantPS("LightBuffer", "lightPos", &lightPos);
	lightShader->updateConstantPS("LightBuffer", "lightColor", &lightColor);
	lightShader->updateConstantPS("LightBuffer", "numLights", &numLight);
	lightShader->updateConstantPS("LightBuffer", "viewPos", &cameraPostion);
	lightShader->applyLight(core);
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

void debugOutput(const std::string& message) {
	OutputDebugStringA(message.c_str());
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
	std::string lightVS = "Resources/Shader/squadVS.hlsl";
	std::string lightPS = "Resources/Shader/lightPixelShader.hlsl";
	std::string shaderName = "MyShader";
	std::string staticShaderName = "staticShader";
	std::string skyShaderName = "skyShader";
	std::string waterShaderName = "waterShader";

	/* defer shading - light shader*/
	Shader* lightShader = new Shader();
	lightShader->loadVS(lightVS, dx);
	lightShader->loadPS(lightPS, dx);
	lightShader->Init(dx->device);
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
	shaders.load(staticShaderName, vs, normalPS, dx);
	shaders.load(waterShaderName, waterVS, normalPS, dx);
	shaders.load(skyShaderName, vs, normalPS, dx);
	Shader* animatedShader = shaders.getShader(shaderName);
	Shader* staticShader = shaders.getShader(staticShaderName);
	Shader* skyShader = shaders.getShader(skyShaderName);
	Shader* waterShader = shaders.getShader(waterShaderName);
	GamesEngineeringBase::Timer tim;
	float t = 0;
	float elapsedTime = 0.0f;
	int frameCount = 0;
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
		dx->geometryPass();
		float dt = tim.dt();
		t += dt;

		elapsedTime += dt;
		frameCount++;
		// update FPS each second
		if (elapsedTime >= 1.0f) {
			float fps = frameCount / elapsedTime;
			frameCount = 0;
			elapsedTime = 0.0f;

			std::string message = "FPS: " + std::to_string(fps) + "\n";
			debugOutput(message);
		}

		// draw sky dome
		sky.update(t);
		sky.draw(dx, skyShader, textures, sam, camera.position, vp);

		handleInput(player, camera, canvas, dt, cube.boundingBox);
		mathLib::Matrix cv = camera.getViewMatrix();
		vp = cv * p;

		// world Matrix
		mathLib::Matrix cubeWorld = mathLib::Matrix::translation(mathLib::Vec3(13.f, 1.f, 0.f));
		mathLib::Matrix waterWorld = mathLib::Matrix::translation(mathLib::Vec3(0.f, 1.f, 0.f));

		cube.updateBoundingBox(cubeWorld);
		cube.draw(dx, staticShader, textures, sam, cubeWorld, vp);
		pl.draw(dx, staticShader, textures, sam, planeWorld, vp);
		grasses.draw(dx, textures, staticShader, sam, vp);
		trees.draw(dx, textures, staticShader, sam, vp);
		player.draw(dx, animatedShader, textures, sam, vp);
		pool.draw(dx, staticShader, textures, sam, vp);
		renderWater(t, water, waterShader, waterWorld, vp, dx, textures, sam);

		/* defer Shading implementation*/
		renderLight(dx, lightShader, camera.position);
		dx->lightingPass();

		canvas.processMessages();
		dx->present();
	}

	textures.unload("Resources/Textures/T-rex_Base_Color.png");
	textures.unload("Resources/Textures/MaleDuty_3_OBJ_Happy_Packed0_Diffuse.png");
}
