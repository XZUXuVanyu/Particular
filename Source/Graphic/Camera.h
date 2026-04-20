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

	void update(GLdouble dt, juce::Point<GLint> new_mouse_pos);
	void onWindowResize(const juce::Rectangle<GLint> new_window_size);

	void setViewDist(GLfloat near, GLfloat far);

	glm::mat4 getGlobalVP();
	glm::vec3 getCameraPos();
	juce::Point<GLint> getMousePos();
	juce::Point<GLfloat> getMouseNDCPos();
	juce::Rectangle<GLint> getWindowSize();

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

	GLfloat accl_a = 10.0, friction_a = 0.95, damp_a = 0.7;
	Quaternion camera_rotation{ 1.0, 0.0,0.0,0.0 };
	glm::vec3 camera_velo_a{ 0.0, 0.0, 0.0 };

	Quaternion camera_r{ 0.0, 1.0,0.0,0.0 };
	Quaternion camera_f{ 0.0, 0.0,1.0,0.0 };
	Quaternion camera_u{ 0.0, 0.0,0.0,1.0 };
	void update_RFU();

	/* Global transform matrices */
	glm::mat4 view_mat;
	glm::mat4 projection_mat;
	void update_Vmat();
	void update_Pmat();

	/* Mouse control  */
	GLfloat sensitivity_x = 0.01;
	GLfloat sensitivity_y = 0.01;
	juce::Point<GLint> mouse_pos{ 0,0 };
	juce::Rectangle<GLint> window_size{ 1920, 1080 };
};
//==============================================================================