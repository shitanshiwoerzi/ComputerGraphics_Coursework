#include "window.h"
#include "dxCore.h"
#include "shader.h"
#include "mesh.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
	Window canvas;
	DxCore dx;
	Shader shader;
	Triangle t;
	t.Init();
	canvas.Init("MyWindow", 1024, 768);
	dx.Init(1024, 768, canvas.hwnd);
	std::string s = "Resources/vertex_shader.txt";
	shader.loadVS(s, dx.device);
	shader.loadPS(s, dx.device);
	t.createBuffer(dx.device);
	while (true) {
		dx.clear();
		canvas.processMessages();
		shader.apply(dx.devicecontext);
		t.draw(dx.devicecontext);
		dx.present();
	}
}