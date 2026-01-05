#pragma once

#include "core/base.h"

namespace penumbra::Math
{
	// Old using declarations
	// Just keeping this here for reference;
	// No longer being used!
	//using Float2 = glm::vec2;
	//using Float3 = glm::vec3;
	//using Float4 = glm::vec4;
	//using Quat = glm::quat;
	//using Mat4 = glm::mat4;
	//using Mat3 = glm::mat3;

	bool DecomposeTransform(const glm::mat4& matTransform, glm::vec3& position, glm::vec3& rotation, glm::vec3& scale);
	glm::mat4 WorldToLocalTransform(const glm::mat4& matWorldTransform, const glm::mat4& matParentTransform);
}