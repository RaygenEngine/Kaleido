#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "asset/loaders/ShaderLoader.h"

#include <glad/glad.h>

namespace OpenGL {
struct GLShader : GLAsset<ShaderPod> {
	GLuint id{ 0 };
	bool firstLoad{ true };

	GLShader(PodHandle<ShaderPod> handle)
		: GLAsset(handle)
	{
	}

	~GLShader() override;

	[[nodiscard]] GLint GetUniform(const std::string& uniformName) const;
	void AddUniform(const std::string& uniformName);
	bool Load() override;

protected:
	// temporary
	std::unordered_map<std::string, GLint> uniformLocations;
};

} // namespace OpenGL
