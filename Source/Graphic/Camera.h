//==============================================================================
/* Camera class for managing view */
//==============================================================================
#pragma once
#include <JuceHeader.h>
#include "../Math/Quaternion.h"
//==============================================================================
class Camera
{
public:
	Camera() = default;
	Camera(GLfloat fov, GLfloat aspect_ratio);
	~Camera() = default;

	void update(GLdouble dt);
	void processMouseMove(const juce::MouseEvent& event);
	void processKeyPress(const juce::KeyPress& key);
	void processWindowResize(const juce::Rectangle<GLint> new_window_size);

	glm::mat4 getGlobalVP();
	glm::vec3 getCameraPos();
	void setLastMousePos(juce::Point<GLint> pos);
	void setViewDist(GLfloat near, GLfloat far);
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Camera);

	/* Camera properties */
	GLfloat fov = 80.0;
	GLfloat dnear = 0.01, dfar = 1000.0;
	GLfloat aspect_ratio = 16.0 / 9.0;

	/* Global coordinate params */
	GLfloat accl_t = 10.0, friction_t = 0.95, damp_t = 0.5;
	glm::vec3 camera_pos{ 0.0, -5.0, 2.0 };
	glm::vec3 camera_velo{ 0.0, 0.0, 0.0 };


	Quaternion camera_r{ 0.0, 1.0,0.0,0.0 };
	Quaternion camera_f{ 0.0, 0.0,1.0,0.0 };
	Quaternion camera_u{ 0.0, 0.0,0.0,1.0 };
	Quaternion camera_rotation{ 1.0, 0.0,0.0,0.0 };

	/* Global transform matrices */
	glm::mat4 view_mat;
	glm::mat4 projection_mat;
	void update_Vmat();
	void update_Pmat();

	/* Mouse control  */
	GLfloat sensitivity_x = 0.01;
	GLfloat sensitivity_y = 0.01;
	juce::Point<GLint> last_mouse_pos{ 0,0 };
};
//==============================================================================