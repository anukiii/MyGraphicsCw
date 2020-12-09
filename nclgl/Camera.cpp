# include "Camera.h"
# include <algorithm>
#include "Window.h"


//Updates camera based on player input
void Camera::UpdateCamera(float dt) {


	pitch -= (Window::GetMouse() -> GetRelativePosition().y); // MOUSE POISION FOR X AND Y
	yaw -= (Window::GetMouse() -> GetRelativePosition().x);
	pitch = std::min(pitch, 90.0f);
	pitch = std::max(pitch, -90.0f);						//END OF MOUSE CAPTURE
	

	if (yaw < 0) {							//RESETS YAW ONCE FULL ROTATION COMPLETE
	yaw += 360.0f;
	
	}
	 if (yaw > 360.0f) {
	yaw -= 360.0f;
	
	}										//END YAW RESETS
	


	Matrix4 rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));		//MATRIXES FOR MOVEMENT + SPEED
	Vector3 forward = rotation * Vector3(0, 0, -1);
	Vector3 right = rotation * Vector3(1, 0, 0);
	
	float speed = 300.0f * dt;								//END MOVEMENT + SPEED


	
	if (Window::GetKeyboard() -> KeyDown(KEYBOARD_W)) {		//WASD CAMERA MOVEMENT
		position += forward * speed;
		
	}
	if (Window::GetKeyboard() -> KeyDown(KEYBOARD_S)) {
		position -= forward * speed;
		
	}
	if (Window::GetKeyboard() -> KeyDown(KEYBOARD_A)) {
		position -= right * speed;
		
	}
	if (Window::GetKeyboard() -> KeyDown(KEYBOARD_D)) { 
		position += right * speed;
		
	}
	if (Window::GetKeyboard() -> KeyDown(KEYBOARD_SHIFT)) {
		position.y += speed;
		
	}
	if (Window::GetKeyboard() -> KeyDown(KEYBOARD_SPACE)) {
		position.y -= speed;
		
	}														//END WASD MOVEMENT
	
}

void Camera::CameraPath(float dt) {

	Matrix4 rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));		//MATRIXES FOR MOVEMENT + SPEED
	Vector3 forward = rotation * Vector3(0, 0, -1);
	Vector3 right = rotation * Vector3(1, 0, 0);
	float speed = 100.0f * dt;

	
	
	if (direction ==1) {
	if (yaw < -30.0f) {							//RESETS YAW ONCE FULL ROTATION COMPLETE
		yaw =-30.0f;

	}
	if (yaw > 30.0f) {
		yaw = 30.0f;

	}
	if (yaw == -30.0f || yaw == 30.0f) {
		yawSway *= -1;
	}
	yaw += yawSway;
	pitch += yawSway;




	pitch = std::min(pitch, 90.0f);
	pitch = std::max(pitch, -90.0f);

	position += forward * speed;
	if (position.z < 4400) {
		direction = -1;
	}
	}
	if (direction ==-1) {
		if (yaw < 30.0f) {							//RESETS YAW ONCE FULL ROTATION COMPLETE
			yaw = 30.0f;

		}
		if (yaw > 60.0f) {
			yaw = 60.0f;

		}
		if (yaw == 30.0f || yaw == 60.0f) {
			yawSway *= -1;
		}
		yaw += yawSway;

		//if (pitch < 50) {
			pitch -= yawSway;
		//}
		position -= forward * speed;


	}

}


//Creates matrix used for shaders, inverse of camera's position and rotation
Matrix4 Camera::BuildViewMatrix() {
	return  Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) *
			Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) * //ORDER IS IMPORTANT
			Matrix4::Translation(-position);
	
};

