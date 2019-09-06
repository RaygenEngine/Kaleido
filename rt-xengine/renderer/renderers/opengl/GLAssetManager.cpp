#include "pch.h"

#include "renderer/renderers/opengl/GLAssetManager.h"

namespace OpenGL
{
	std::shared_ptr<GLCubeMap> GLAssetManager::RequestGLCubeMap(CubeMap* cubeMap, GLint wrapFlag,
		bool mipMapping)
	{
		return LoadAssetAtMultiKeyCache<GLCubeMap>(m_glCubeMaps, this, cubeMap->GetFileName(), cubeMap, wrapFlag, mipMapping);
	}

	std::shared_ptr<GLTexture> GLAssetManager::RequestGLTexture(Texture* texture, GLint minFilter,
		GLint magFilter, GLint wrapS, GLint wrapT, GLint wrapR)
	{
		return LoadAssetAtMultiKeyCache<GLTexture>(m_glTextures, this, texture->GetFileName(), texture, minFilter, magFilter, wrapS, wrapT, wrapR);
	}

	std::shared_ptr<GLShader> GLAssetManager::RequestGLShader(StringFile* vertexFile, StringFile* fragmentFile)
	{
		const auto name = "vert> " + vertexFile->GetFileName() + "frag> " + fragmentFile->GetFileName();

		return LoadAssetAtMultiKeyCache<GLShader>(m_glShaders, this, name, vertexFile, fragmentFile);
	}

	std::shared_ptr<GLModel> GLAssetManager::RequestGLModel(Model* model)
	{
		return LoadAssetAtMultiKeyCache<GLModel>(m_glModels, this, model->GetFileName(), model);
	}

	// TODO: caching may not work correctly with instancing, check this out
	//std::shared_ptr<GLInstancedModel> GLRendererBase::RequestGLInstancedModel(World::TriangleModelInstancedGeometryNode* nodeInstancer)
	//{
	//	const auto name = nodeInstancer->GetModel()->GetName();

	//	return Assets::LoadAssetAtMultiKeyCache<GLInstancedModel>(m_glInstancedModels, this, name, nodeInstancer);
	//}
}
