#include "pch.h"
#include "GLCubeMap.h"

namespace Renderer::OpenGL
{

	GLCubeMap::GLCubeMap(GLRendererBase* renderer)
		: GLAsset(renderer),
		  m_cubeTextureId(0)
	{
	}

	GLCubeMap::~GLCubeMap()
	{
		glDeleteTextures(1, &m_cubeTextureId);
	}

	bool GLCubeMap::Load(Assets::CubeMap* data, GLint wrapFlag, bool mipMapping)
	{
		SetIdentificationFromAssociatedDiskAssetIdentification(data->GetLabel());

		glGenTextures(1, &m_cubeTextureId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeTextureId);

		if (mipMapping)
		{
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		GLenum type;
		GLint internalFormat;
		switch (data->GetType())
		{

			// considered hdr - shader doesn't normalize
		case Assets::DR_HIGH:
			type = GL_FLOAT;
			internalFormat = GL_RGBA32F;
			break;

		case Assets::DR_LOW:
		default:
			type = GL_UNSIGNED_BYTE;
			internalFormat = GL_RGBA;
			break;
		}

		// format is RGBA (stb)
		const GLenum format = GL_RGBA;

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapFlag);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapFlag);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapFlag);


		for (auto i = 0; i < Assets::CMF_COUNT; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat,
				data->GetWidth(), data->GetHeight(), 0, format, type, data->GetFace(static_cast<Assets::CUBE_MAP_FACE>(i))->GetData());
		}

		
		return true;
	}
}
