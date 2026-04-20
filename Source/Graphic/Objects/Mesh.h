#pragma once
#include <JuceHeader.h>
#include "../Renderer.h"
//==============================================================================
/* A renderer that organizes different Objects */
class Mesh : public Object
{
public:
	using Object::Object;

	void initialise() override;
	void render(const glm::mat4& global_VP, const glm::vec3& camera_pos) override;
	void cleanup() override;
private:
};