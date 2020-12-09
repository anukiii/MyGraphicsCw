#include "../nclgl/window.h"
#include "Renderer.h"

int main() {
	Window w("CyberPunk 2078", 1280, 720, false);
	if (!w.HasInitialised()) {
		return -1;
	}

	Renderer renderer(w);
	if (!renderer.HasInitialised()) {
		std::cin.ignore();

		return -1;
	}

	w.LockMouseToWindow(true);
	w.ShowOSPointer(false);


	while (w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_Q) && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
		renderer.UpdateSceneAuto(w.GetTimer()->GetTimeDeltaSeconds());
		renderer.RenderScene();
		renderer.SwapBuffers();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_0)) {
			renderer.SetPostProcessingType(0);
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_1)) {
			renderer.SetPostProcessingType(1);
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_2)) {
			renderer.SetPostProcessingType(2);
		}		
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_3)) {
			renderer.SetPostProcessingType(3);
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
			Shader::ReloadAllShaders();
		}
	}
	while (w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {

		renderer.UpdateScene(w.GetTimer()->GetTimeDeltaSeconds());
		renderer.RenderScene();
		renderer.SwapBuffers();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_0)) {
			renderer.SetPostProcessingType(0);
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_1)) {
			renderer.SetPostProcessingType(1);
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_2)) {
			renderer.SetPostProcessingType(2);
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_3)) {
			renderer.SetPostProcessingType(3);
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
			Shader::ReloadAllShaders();
		}
	}

	return 0;
}