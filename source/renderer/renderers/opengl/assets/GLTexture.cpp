#include "pch/pch.h"

#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/GLUtil.h"
#include "renderer/renderers/opengl/GLPreviewer.h"
#include "asset/AssetManager.h"
#include "asset/pods/TexturePod.h"

namespace ogl {
GLTexture::~GLTexture()
{
	glDeleteTextures(1, &id);
}

void GLTexture::Load()
{
	const auto textureData = podHandle.Lock();

	glGenTextures(1, &id);

	const GLint textureTarget = GetGLTextureTarget(textureData->target);

	glBindTexture(textureTarget, id);

	const auto minFiltering = GetGLFiltering(textureData->minFilter);

	glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GetGLFiltering(textureData->magFilter));
	glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, minFiltering);
	glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GetGLWrapping(textureData->wrapS));
	glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GetGLWrapping(textureData->wrapT));
	glTexParameteri(textureTarget, GL_TEXTURE_WRAP_R, GetGLWrapping(textureData->wrapR));

	struct TextureSetup {
		GLint internalformat{ GL_INVALID_VALUE };
		GLenum format{ GL_INVALID_ENUM };
		GLenum type{ GL_INVALID_ENUM };
	};

	const auto GetTextureSetup = [&](bool isLinear, bool isHdr, int32 components) -> TextureSetup {
		TextureSetup ts{};
		ts.internalformat = (!isHdr ? (isLinear ? GL_RGBA : GL_SRGB8_ALPHA8) : (isLinear ? GL_RGBA32F : GL_SRGB_ALPHA));
		ts.format = GL_RGBA;
		ts.type = (!isHdr ? GL_UNSIGNED_BYTE : GL_FLOAT);
		return ts;
	};

	switch (textureData->target) {
		case TextureTarget::TEXTURE_2D: {
			const auto img = textureData->images.at(0);

			auto imgPod = img.Lock();

			const auto textureSetup = GetTextureSetup(textureData->isLinear, imgPod->isHdr, imgPod->components);
			glTexImage2D(GL_TEXTURE_2D, 0, textureSetup.internalformat, imgPod->width, imgPod->height, 0,
				textureSetup.format, textureSetup.type, imgPod->data);

			GetGLPreviewer(this)->AddPreview(id, AssetManager::GetEntry(img)->name);

			break;
		}

		case TextureTarget::TEXTURE_CUBEMAP: {
			for (auto i = 0; i < CubemapFace::COUNT; ++i) {
				const auto img = textureData->images.at(i);

				auto imgPod = img.Lock();

				const auto textureSetup = GetTextureSetup(textureData->isLinear, imgPod->isHdr, imgPod->components);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, textureSetup.internalformat, imgPod->width,
					imgPod->height, 0, textureSetup.format, textureSetup.type, imgPod->data);
			}
			break;
		}
		case TextureTarget::TEXTURE_1D:
		case TextureTarget::TEXTURE_3D:
		case TextureTarget::TEXTURE_ARRAY:
		case TextureTarget::TEXTURE_CUBEMAP_ARRAY:
		default: LOG_ABORT("Texture format yet supported");
	}

	if (minFiltering == GL_NEAREST_MIPMAP_NEAREST || minFiltering == GL_LINEAR_MIPMAP_NEAREST
		|| minFiltering == GL_NEAREST_MIPMAP_LINEAR || minFiltering == GL_LINEAR_MIPMAP_LINEAR) {

		glGenerateMipmap(textureTarget);
	}
}
} // namespace ogl
