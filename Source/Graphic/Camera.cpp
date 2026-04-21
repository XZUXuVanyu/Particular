//==============================================================================
#include "Camera.h"
#include <glm-master/glm/glm.hpp>
#include <glm-master/glm/gtc/matrix_transform.hpp>
//==============================================================================
Camera::Camera(GLfloat fov, GLfloat aspect_ratio) : fov(fov), aspect_ratio(aspect_ratio)
{
	update_Pmat();
	update_Vmat();
}
void Camera::update(GLdouble dt, juce::Point<GLint> new_mouse_pos)
{
	GLfloat dt_f = (GLfloat)dt;
	glm::vec3 target_direction{ 0.0, 0.0, 0.0 };
	if (juce::ModifierKeys::getCurrentModifiers().isShiftDown()) accl_t = 25.0f;
	else accl_t = 10.0f;
	if (juce::KeyPress::isKeyCurrentlyDown('W')) target_direction += camera_f.v;
	if (juce::KeyPress::isKeyCurrentlyDown('S')) target_direction -= camera_f.v;
	if (juce::KeyPress::isKeyCurrentlyDown('A')) target_direction -= camera_r.v;
	if (juce::KeyPress::isKeyCurrentlyDown('D')) target_direction += camera_r.v;
	if (juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::spaceKey)) target_direction += camera_u.v;
	
	camera_velo += target_direction * accl_t * dt_f;
	camera_velo *= pow(friction_t, (GLfloat)60.0f * damp_t * dt_f);
	camera_pos	+= camera_velo * dt_f;

	if (juce::ModifierKeys::getCurrentModifiers().isLeftButtonDown())
	{
		camera_velo_a.x += -(new_mouse_pos.y - mouse_pos.y) * sensitivity_y * accl_a * dt_f;
		camera_velo_a.z += -(new_mouse_pos.x - mouse_pos.x) * sensitivity_x * accl_a * dt_f;
	}
	mouse_pos = new_mouse_pos;
	camera_velo_a *= pow(friction_a, (GLfloat)60.0f * damp_a * dt_f);
	camera_rotation = Quaternion::gen_rotater(camera_velo_a.z, glm::vec3{ 0,0,1 }) * camera_rotation;
	camera_rotation = camera_rotation * Quaternion::gen_rotater(camera_velo_a.x, glm::vec3{ 1,0,0 });
	Quaternion::normalize(camera_rotation);
	update_RFU();
	update_Vmat();
}
void Camera::onWindowResize(const juce::Rectangle<GLint> new_window_size)
{
	GLfloat pi = juce::MathConstants<GLfloat>::pi;
	if (new_window_size.getWidth() == 0 || new_window_size.getHeight() == 0)
	{
		DBG("[INFO] Window size = 0, may caused by an error");
		return;
	}
	window_size = new_window_size;
	aspect_ratio = (GLfloat)window_size.getWidth() / (GLfloat)window_size.getHeight();
	sensitivity_x = pi / (GLfloat)window_size.getWidth();
	sensitivity_y = pi / (GLfloat)window_size.getHeight();
	update_Pmat();
}
glm::mat4 Camera::getGlobalVP()
{
	return projection_mat * view_mat;
}
glm::vec3 Camera::getCameraPos()
{
	return camera_pos;
}
juce::Point<GLint> Camera::getMousePos()
{
	return mouse_pos;
}
juce::Point<GLfloat> Camera::getMouseNDCPos()
{
	GLfloat w = (GLfloat)window_size.getWidth();
	GLfloat h = (GLfloat)window_size.getHeight();

	if (w <= 0.0f || h <= 0.0f) return { 0.0f, 0.0f };

	GLfloat nx = (2.0f * (GLfloat)mouse_pos.x / w) - 1.0f;
	GLfloat ny = 1.0f - (2.0f * (GLfloat)mouse_pos.y / h);

	return { nx, ny };
}
juce::Rectangle<GLint> Camera::getWindowSize()
{
	return window_size;
}
void Camera::setViewDist(GLfloat near, GLfloat far)
{
	dnear = near;
	dfar = far;
}
void Camera::update_RFU()
{
	Quaternion inv_q{ camera_rotation.get_conjugate() };
	camera_r = (camera_rotation * Quaternion{ 0.0, glm::vec3{1,0,0} } * inv_q);
	camera_f = (camera_rotation * Quaternion{ 0.0, glm::vec3{0,1,0} } * inv_q);
	camera_u = (camera_rotation * Quaternion{ 0.0, glm::vec3{0,0,1} } * inv_q);
}
void Camera::update_Vmat()
{
	glm::mat4 rotation_mat = glm::mat4(
		glm::vec4(camera_r.v.x, camera_u.v.x, -camera_f.v.x, 0.0f),
		glm::vec4(camera_r.v.y, camera_u.v.y, -camera_f.v.y, 0.0f),
		glm::vec4(camera_r.v.z, camera_u.v.z, -camera_f.v.z, 0.0f),
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	glm::mat4 traslation_mat = glm::mat4(
		glm::vec4(1, 0, 0, 0),
		glm::vec4(0, 1, 0, 0),
		glm::vec4(0, 0, 1, 0),
		glm::vec4(-camera_pos.x, -camera_pos.y, -camera_pos.z, 1)
	);
	view_mat = rotation_mat * traslation_mat;
}
void Camera::update_Pmat()
{
	projection_mat = glm::perspective(glm::radians(fov), aspect_ratio, dnear, dfar);
}
//==============================================================================