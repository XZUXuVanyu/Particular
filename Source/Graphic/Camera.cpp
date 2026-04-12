#include <JuceHeader.h>
#include "Camera.h"
#include <glm-master/glm/glm.hpp>
#include <glm-master/glm/gtc/matrix_transform.hpp>
//==============================================================================
Camera::Camera(GLfloat fov, GLfloat aspect_ratio) : fov(fov), aspect_ratio(aspect_ratio)
{
	update_Pmat();
	update_Vmat();
}
void Camera::update(GLdouble dt)
{
	GLfloat dt_f = (GLfloat)dt;
	glm::vec3 target_direction{ 0.0, 0.0, 0.0 };
	if (juce::KeyPress::isKeyCurrentlyDown('w')) target_direction += camera_f.v;
	if (juce::KeyPress::isKeyCurrentlyDown('s')) target_direction -= camera_f.v;
	if (juce::KeyPress::isKeyCurrentlyDown('a')) target_direction -= camera_r.v;
	if (juce::KeyPress::isKeyCurrentlyDown('d')) target_direction += camera_r.v;
	if (juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::spaceKey)) target_direction += camera_u.v;
	
	camera_velo += target_direction * accl_t * dt_f;
	camera_velo *= pow(friction_t, (GLfloat)60.0f * damp_t * dt_f);
	camera_pos	+= camera_velo * dt_f;
	update_Vmat();
}
void Camera::processMouseMove(const juce::MouseEvent& event)
{
	auto current_pos = event.getPosition();
	GLfloat delta_x = current_pos.x - last_mouse_pos.x;
	GLfloat delta_y = current_pos.y - last_mouse_pos.y;
	last_mouse_pos = current_pos;

	float lr_angle = -delta_x * sensitivity_x;
	float ud_angle = -delta_y * sensitivity_y;

	camera_rotation = Quaternion::gen_rotater(lr_angle, glm::vec3{ 0,0,1 }) * camera_rotation;
	camera_rotation = camera_rotation * Quaternion::gen_rotater(ud_angle, glm::vec3{ 1,0,0 });
	Quaternion::normalize(camera_rotation);

	Quaternion inv_q{ camera_rotation.get_conjugate() };
	camera_r = (camera_rotation * Quaternion{ 0.0, glm::vec3{1,0,0} } * inv_q);
	camera_f = (camera_rotation * Quaternion{ 0.0, glm::vec3{0,1,0} } * inv_q);
	camera_u = (camera_rotation * Quaternion{ 0.0, glm::vec3{0,0,1} } * inv_q);
	update_Vmat();
}
void Camera::processWindowResize(const juce::Rectangle<GLint> new_window_size)
{
	GLfloat pi = juce::MathConstants<GLfloat>::pi;
	GLfloat width = new_window_size.getWidth(), height = new_window_size.getHeight();
	if (height == 0)
	{
		DBG("[INFO] Window size = 0, may caused by an error");
		return;
	}
	aspect_ratio = width / height;
	sensitivity_x = pi / width, sensitivity_y = pi / height;
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
void Camera::setLastMousePos(juce::Point<GLint> pos)
{
	last_mouse_pos = pos;
}
void Camera::setViewDist(GLfloat near, GLfloat far)
{
	dnear = near;
	dfar = far;
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
