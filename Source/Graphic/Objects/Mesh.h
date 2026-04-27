//==============================================================================
#pragma once
#include "../Object.h"
//==============================================================================
class Mesh : public Object
{
public:
	using Object::Object;
	void	childInitialise() override;
	void	childRender(const glm::mat4& global_VP, const glm::vec3& camera_pos) override;
	void	childCleanup() override;

private:
};
//==============================================================================