#include "pch/pch.h"

#include "renderer/renderers/opengl/GLAssetManager.h"
#include "renderer/renderers/opengl/GLPreviewer.h"
#include "renderer/renderers/opengl/deferred/GLDeferredRenderer.h"
#include "world/World.h"
#include "world/nodes/RootNode.h"
#include "world/nodes/camera/CameraNode.h"
#include "world/nodes/light/AmbientNode.h"
#include "system/Input.h"
#include "platform/windows/Win32Window.h"
#include "renderer/renderers/opengl/GLUtil.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>


namespace ogl {

GLDeferredRenderer::GBuffer::~GBuffer()
{
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &positionsAttachment);
	glDeleteTextures(1, &normalsAttachment);
	glDeleteTextures(1, &albedoOpacityAttachment);
	glDeleteTextures(1, &specularAttachment);
	glDeleteTextures(1, &emissiveAttachment);
	glDeleteTextures(1, &depthAttachment);
}

GLDeferredRenderer::~GLDeferredRenderer()
{
	glDeleteFramebuffers(1, &m_lightFbo);
	glDeleteTextures(1, &m_lightTexture);
	glDeleteFramebuffers(1, &m_outFbo);
	glDeleteTextures(1, &m_outTexture);
}

void GLDeferredRenderer::InitObservers()
{
	m_activeCamera = Engine::GetWorld()->GetActiveCamera();
	CLOG_WARN(!m_activeCamera, "Renderer failed to find camera.");


	const auto reload = [](GLBasicAmbient* ambObs) {
		ambObs->ReloadSkybox();
	};

	m_ambient = CreateTrackerObserver_AnyAvailableWithCallback<GLBasicAmbient>(reload);

	RegisterObserverContainer_AutoLifetimes(m_glGeometries);
	RegisterObserverContainer_AutoLifetimes(m_glDirectionalLights);
	RegisterObserverContainer_AutoLifetimes(m_glSpotLights);
	RegisterObserverContainer_AutoLifetimes(m_glPunctualLights);
}

void GLDeferredRenderer::InitShaders()
{
	m_gBuffer.shader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/deferred/DR_GBuffer_AlphaMask.json");

	m_gBuffer.shader->StoreUniformLoc("mvp");
	m_gBuffer.shader->StoreUniformLoc("m");
	m_gBuffer.shader->StoreUniformLoc("normalMatrix");
	// material
	m_gBuffer.shader->StoreUniformLoc("material.baseColorFactor");
	m_gBuffer.shader->StoreUniformLoc("material.emissiveFactor");
	m_gBuffer.shader->StoreUniformLoc("material.metallicFactor");
	m_gBuffer.shader->StoreUniformLoc("material.roughnessFactor");
	m_gBuffer.shader->StoreUniformLoc("material.normalScale");
	m_gBuffer.shader->StoreUniformLoc("material.occlusionStrength");
	m_gBuffer.shader->StoreUniformLoc("material.baseColorTexcoordIndex");
	m_gBuffer.shader->StoreUniformLoc("material.baseColorSampler");
	m_gBuffer.shader->StoreUniformLoc("material.metallicRoughnessTexcoordIndex");
	m_gBuffer.shader->StoreUniformLoc("material.metallicRoughnessSampler");
	m_gBuffer.shader->StoreUniformLoc("material.emissiveTexcoordIndex");
	m_gBuffer.shader->StoreUniformLoc("material.emissiveSampler");
	m_gBuffer.shader->StoreUniformLoc("material.normalTexcoordIndex");
	m_gBuffer.shader->StoreUniformLoc("material.normalSampler");
	m_gBuffer.shader->StoreUniformLoc("material.occlusionTexcoordIndex");
	m_gBuffer.shader->StoreUniformLoc("material.occlusionSampler");
	m_gBuffer.shader->StoreUniformLoc("material.alphaCutoff");
	m_gBuffer.shader->StoreUniformLoc("material.mask");

	m_deferredDirectionalLightShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/deferred/DR_DirectionalLight.json");

	m_deferredDirectionalLightShader->StoreUniformLoc("wcs_viewPos");

	// directional light
	m_deferredDirectionalLightShader->StoreUniformLoc("directionalLight.wcs_dir");
	m_deferredDirectionalLightShader->StoreUniformLoc("directionalLight.color");
	m_deferredDirectionalLightShader->StoreUniformLoc("directionalLight.intensity");
	m_deferredDirectionalLightShader->StoreUniformLoc("directionalLight.mvpBiased");
	m_deferredDirectionalLightShader->StoreUniformLoc("directionalLight.maxShadowBias");
	m_deferredDirectionalLightShader->StoreUniformLoc("directionalLight.samples");
	m_deferredDirectionalLightShader->StoreUniformLoc("directionalLight.shadowMap");
	m_deferredDirectionalLightShader->StoreUniformLoc("directionalLight.castsShadow");

	// gBuffer
	m_deferredDirectionalLightShader->StoreUniformLoc("gBuffer.positionsSampler");
	m_deferredDirectionalLightShader->StoreUniformLoc("gBuffer.normalsSampler");
	m_deferredDirectionalLightShader->StoreUniformLoc("gBuffer.albedoOpacitySampler");
	m_deferredDirectionalLightShader->StoreUniformLoc("gBuffer.specularSampler");
	m_deferredDirectionalLightShader->StoreUniformLoc("gBuffer.depthSampler");

	m_deferredSpotLightShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/deferred/DR_SpotLight.json");

	m_deferredSpotLightShader->StoreUniformLoc("wcs_viewPos");

	// spot light
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.wcs_pos");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.wcs_dir");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.outerCutOff");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.innerCutOff");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.color");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.intensity");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.attenCoef");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.mvpBiased");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.samples");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.maxShadowBias");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.shadowMap");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.castsShadow");

	// gBuffer
	m_deferredSpotLightShader->StoreUniformLoc("gBuffer.positionsSampler");
	m_deferredSpotLightShader->StoreUniformLoc("gBuffer.normalsSampler");
	m_deferredSpotLightShader->StoreUniformLoc("gBuffer.albedoOpacitySampler");
	m_deferredSpotLightShader->StoreUniformLoc("gBuffer.specularSampler");
	m_deferredSpotLightShader->StoreUniformLoc("gBuffer.depthSampler");

	m_deferredPunctualLightShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/deferred/DR_PunctualLight.json");

	m_deferredPunctualLightShader->StoreUniformLoc("wcs_viewPos");

	// punctual light
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.wcs_pos");
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.color");
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.intensity");
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.far");
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.attenCoef");
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.samples");
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.maxShadowBias");
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.shadowCubemap");
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.castsShadow");

	// gBuffer
	m_deferredPunctualLightShader->StoreUniformLoc("gBuffer.positionsSampler");
	m_deferredPunctualLightShader->StoreUniformLoc("gBuffer.normalsSampler");
	m_deferredPunctualLightShader->StoreUniformLoc("gBuffer.albedoOpacitySampler");
	m_deferredPunctualLightShader->StoreUniformLoc("gBuffer.specularSampler");
	m_deferredPunctualLightShader->StoreUniformLoc("gBuffer.depthSampler");


	m_ambientLightShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/deferred/DR_AmbientLight.json");

	m_ambientLightShader->StoreUniformLoc("wcs_viewPos");
	m_ambientLightShader->StoreUniformLoc("vp_inv");
	m_ambientLightShader->StoreUniformLoc("ambient");

	m_dummyPostProcShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/post-process/DummyPostProc.json");
	m_dummyPostProcShader->StoreUniformLoc("gamma");
	m_dummyPostProcShader->StoreUniformLoc("exposure");

	m_windowShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/general/QuadWriteTexture.json");
}

void GLDeferredRenderer::InitRenderBuffers()
{
	const auto width = m_activeCamera->GetWidth();
	const auto height = m_activeCamera->GetHeight();

	glGenFramebuffers(1, &m_gBuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer.fbo);

	// - rgb: position
	glGenTextures(1, &m_gBuffer.positionsAttachment);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer.positionsAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gBuffer.positionsAttachment, 0);

	// - rgb: normal
	glGenTextures(1, &m_gBuffer.normalsAttachment);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer.normalsAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gBuffer.normalsAttachment, 0);

	// - rgb: albedo, a: opacity
	glGenTextures(1, &m_gBuffer.albedoOpacityAttachment);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer.albedoOpacityAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_gBuffer.albedoOpacityAttachment, 0);

	// - r: metallic, g: roughness, b: occlusion, a: occlusion strength
	glGenTextures(1, &m_gBuffer.specularAttachment);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer.specularAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_gBuffer.specularAttachment, 0);

	// - rgb: emissive, a: <reserved>
	glGenTextures(1, &m_gBuffer.emissiveAttachment);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer.emissiveAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, m_gBuffer.emissiveAttachment, 0);

	GLuint attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(5, attachments);

	glGenTextures(1, &m_gBuffer.depthAttachment);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer.depthAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_gBuffer.depthAttachment, 0);

	CLOG_ABORT(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not complete!");

	// light fbo
	glGenFramebuffers(1, &m_lightFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_lightFbo);

	glGenTextures(1, &m_lightTexture);
	glBindTexture(GL_TEXTURE_2D, m_lightTexture);
	// HDR
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_lightTexture, 0);

	CLOG_ABORT(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not complete!");

	// out fbo
	glGenFramebuffers(1, &m_outFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_outFbo);

	glGenTextures(1, &m_outTexture);
	glBindTexture(GL_TEXTURE_2D, m_outTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outTexture, 0);

	CLOG_ABORT(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not complete!");


	GetGLPreviewer()->AddPreview(m_gBuffer.positionsAttachment, "positions");
	GetGLPreviewer()->AddPreview(m_gBuffer.normalsAttachment, "normals");
	GetGLPreviewer()->AddPreview(m_gBuffer.albedoOpacityAttachment, "albedoOpacity");
	GetGLPreviewer()->AddPreview(m_gBuffer.specularAttachment, "specular");
	GetGLPreviewer()->AddPreview(m_gBuffer.emissiveAttachment, "emissive");
	GetGLPreviewer()->AddPreview(m_gBuffer.depthAttachment, "depth");
	GetGLPreviewer()->AddPreview(m_lightTexture, "light texture");
}

void GLDeferredRenderer::InitScene()
{
	GLEditorRenderer::InitScene();

	InitObservers();

	InitShaders();

	InitRenderBuffers();
}

void GLDeferredRenderer::ClearBuffers()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_lightFbo);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_outFbo);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer.fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void GLDeferredRenderer::RenderGBuffer()
{
	glViewport(0, 0, m_activeCamera->GetWidth(), m_activeCamera->GetHeight());

	glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer.fbo);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	auto gs = m_gBuffer.shader;
	glUseProgram(gs->programId);

	const auto vp = m_activeCamera->GetViewProjectionMatrix();

	// render geometry (non-instanced)
	for (auto& geometry : m_glGeometries) {
		// view frustum culling
		if (!m_activeCamera->IsNodeInsideFrustum(geometry->node)) {
			continue;
		}

		auto m = geometry->node->GetNodeTransformWCS();
		auto mvp = vp * m;

		gs->SendMat4("m", m);
		gs->SendMat4("mvp", mvp);
		gs->SendMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(m))));

		for (auto& glMesh : geometry->glModel->meshes) {
			GLMaterial* glMaterial = glMesh.material;
			const MaterialPod* materialData = glMaterial->LockData();

			// material
			gs->SendVec4("material.baseColorFactor", materialData->baseColorFactor);
			gs->SendVec3("material.emissiveFactor", materialData->emissiveFactor);
			gs->SendFloat("material.metallicFactor", materialData->metallicFactor);
			gs->SendFloat("material.roughnessFactor", materialData->roughnessFactor);
			gs->SendFloat("material.normalScale", materialData->normalScale);
			gs->SendFloat("material.occlusionStrength", materialData->occlusionStrength);
			gs->SendFloat("material.alphaCutoff", materialData->alphaCutoff);
			gs->SendInt("material.mask", materialData->alphaMode == MaterialPod::MASK ? GL_TRUE : GL_FALSE);
			gs->SendInt("material.baseColorTexcoordIndex", materialData->baseColorTexCoordIndex);
			gs->SendInt("material.metallicRoughnessTexcoordIndex", materialData->metallicRoughnessTexCoordIndex);
			gs->SendInt("material.emissiveTexcoordIndex", materialData->emissiveTexCoordIndex);
			gs->SendInt("material.normalTexcoordIndex", materialData->normalTexCoordIndex);
			gs->SendInt("material.occlusionTexcoordIndex", materialData->occlusionTexCoordIndex);

			gs->SendTexture("material.baseColorSampler", glMaterial->baseColorTexture->id, 0);
			gs->SendTexture("material.metallicRoughnessSampler", glMaterial->metallicRoughnessTexture->id, 1);
			gs->SendTexture("material.emissiveSampler", glMaterial->emissiveTexture->id, 2);
			gs->SendTexture("material.normalSampler", glMaterial->normalTexture->id, 3);
			gs->SendTexture("material.occlusionSampler", glMaterial->occlusionTexture->id, 4);

			materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

			glBindVertexArray(glMesh.vao);
			report_glDrawElements(GL_TRIANGLES, glMesh.indicesCount, GL_UNSIGNED_INT, (GLvoid*)0);
		}
	}
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void GLDeferredRenderer::RenderDirectionalLights()
{
	auto ls = m_deferredDirectionalLightShader;

	for (auto light : m_glDirectionalLights) {

		// light AABB camera frustum culling
		if (!m_activeCamera->IsNodeInsideFrustum(light->node)) {
			continue;
		}

		light->RenderShadowMap(m_glGeometries);

		glViewport(0, 0, m_activeCamera->GetWidth(), m_activeCamera->GetHeight());

		glBindFramebuffer(GL_FRAMEBUFFER, m_lightFbo);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		glUseProgram(ls->programId);

		// global uniforms
		ls->SendVec3("wcs_viewPos", m_activeCamera->GetNodePositionWCS());

		// light
		ls->SendVec3("directionalLight.wcs_dir", light->node->GetNodeForwardWCS());
		ls->SendVec3("directionalLight.color", light->node->GetColor());
		ls->SendFloat("directionalLight.intensity", light->node->GetIntensity());
		ls->SendInt("directionalLight.samples", light->node->GetSamples());
		ls->SendInt("directionalLight.castsShadow", light->node->HasShadow() ? GL_TRUE : GL_FALSE);
		ls->SendFloat("directionalLight.maxShadowBias", light->node->GetMaxShadowBias());
		ls->SendTexture("directionalLight.shadowMap", light->shadowMap, 0);
		constexpr glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0);
		glm::mat4 mvpBiased = biasMatrix * light->node->GetViewProjectionMatrix();
		ls->SendMat4("directionalLight.mvpBiased", mvpBiased);

		// gBuffer
		ls->SendTexture("gBuffer.positionsSampler", m_gBuffer.positionsAttachment, 1);
		ls->SendTexture("gBuffer.normalsSampler", m_gBuffer.normalsAttachment, 2);
		ls->SendTexture("gBuffer.albedoOpacitySampler", m_gBuffer.albedoOpacityAttachment, 3);
		ls->SendTexture("gBuffer.specularSampler", m_gBuffer.specularAttachment, 4);
		ls->SendTexture("gBuffer.depthSampler", m_gBuffer.depthAttachment, 5);

		// big triangle trick, no vao
		report_glDrawArrays(GL_TRIANGLES, 0, 3);

		glDisable(GL_BLEND);
	}
}

void GLDeferredRenderer::RenderSpotLights()
{
	auto ls = m_deferredSpotLightShader;

	for (auto light : m_glSpotLights) {

		// light AABB camera frustum culling
		if (!m_activeCamera->IsNodeInsideFrustum(light->node)) {
			continue;
		}

		light->RenderShadowMap(m_glGeometries);

		glViewport(0, 0, m_activeCamera->GetWidth(), m_activeCamera->GetHeight());

		glBindFramebuffer(GL_FRAMEBUFFER, m_lightFbo);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		glUseProgram(ls->programId);

		// global uniforms
		ls->SendVec3("wcs_viewPos", m_activeCamera->GetNodePositionWCS());

		// light
		ls->SendVec3("spotLight.wcs_pos", light->node->GetNodePositionWCS());
		ls->SendVec3("spotLight.wcs_dir", light->node->GetNodeForwardWCS());
		ls->SendFloat("spotLight.outerCutOff", glm::cos(light->node->GetOuterAperture() / 2.f));
		ls->SendFloat("spotLight.innerCutOff", glm::cos(light->node->GetInnerAperture() / 2.f));
		ls->SendVec3("spotLight.color", light->node->GetColor());
		ls->SendFloat("spotLight.intensity", light->node->GetIntensity());
		ls->SendInt("spotLight.attenCoef", light->node->GetAttenuationMode());
		ls->SendInt("spotLight.samples", light->node->GetSamples());
		ls->SendInt("spotLight.castsShadow", light->node->HasShadow() ? GL_TRUE : GL_FALSE);
		ls->SendFloat("spotLight.maxShadowBias", light->node->GetMaxShadowBias());
		ls->SendTexture("spotLight.shadowMap", light->shadowMap, 0);
		constexpr glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0);
		glm::mat4 mvpBiased = biasMatrix * light->node->GetViewProjectionMatrix();
		ls->SendMat4("spotLight.mvpBiased", mvpBiased);

		// gBuffer
		ls->SendTexture("gBuffer.positionsSampler", m_gBuffer.positionsAttachment, 1);
		ls->SendTexture("gBuffer.normalsSampler", m_gBuffer.normalsAttachment, 2);
		ls->SendTexture("gBuffer.albedoOpacitySampler", m_gBuffer.albedoOpacityAttachment, 3);
		ls->SendTexture("gBuffer.specularSampler", m_gBuffer.specularAttachment, 4);
		ls->SendTexture("gBuffer.depthSampler", m_gBuffer.depthAttachment, 5);

		// big triangle trick, no vao
		report_glDrawArrays(GL_TRIANGLES, 0, 3);

		glDisable(GL_BLEND);
	}
}

void GLDeferredRenderer::RenderPunctualLights()
{
	auto ls = m_deferredPunctualLightShader;

	for (auto light : m_glPunctualLights) {

		// light AABB camera frustum culling
		if (!m_activeCamera->IsNodeInsideFrustum(light->node)) {
			continue;
		}

		light->RenderShadowMap(m_glGeometries);

		glViewport(0, 0, m_activeCamera->GetWidth(), m_activeCamera->GetHeight());

		glBindFramebuffer(GL_FRAMEBUFFER, m_lightFbo);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		glUseProgram(ls->programId);

		// global uniforms
		ls->SendVec3("wcs_viewPos", m_activeCamera->GetNodePositionWCS());

		// light
		ls->SendVec3("punctualLight.wcs_pos", light->node->GetNodePositionWCS());
		ls->SendVec3("punctualLight.color", light->node->GetColor());
		ls->SendFloat("punctualLight.intensity", light->node->GetIntensity());
		ls->SendFloat("punctualLight.far", light->node->GetFar());
		ls->SendInt("punctualLight.attenCoef", light->node->GetAttenuationMode());
		ls->SendInt("punctualLight.samples", light->node->GetSamples());
		ls->SendInt("punctualLight.castsShadow", light->node->HasShadow() ? GL_TRUE : GL_FALSE);
		ls->SendFloat("punctualLight.maxShadowBias", light->node->GetMaxShadowBias());
		ls->SendCubeTexture("punctualLight.shadowCubemap", light->cubeShadowMap, 0);

		// gBuffer
		ls->SendTexture("gBuffer.positionsSampler", m_gBuffer.positionsAttachment, 1);
		ls->SendTexture("gBuffer.normalsSampler", m_gBuffer.normalsAttachment, 2);
		ls->SendTexture("gBuffer.albedoOpacitySampler", m_gBuffer.albedoOpacityAttachment, 3);
		ls->SendTexture("gBuffer.specularSampler", m_gBuffer.specularAttachment, 4);
		ls->SendTexture("gBuffer.depthSampler", m_gBuffer.depthAttachment, 5);

		// big triangle trick, no vao
		report_glDrawArrays(GL_TRIANGLES, 0, 3);

		glDisable(GL_BLEND);
	}
}

void GLDeferredRenderer::RenderAmbientLight()
{
	glViewport(0, 0, m_activeCamera->GetWidth(), m_activeCamera->GetHeight());

	glBindFramebuffer(GL_FRAMEBUFFER, m_lightFbo);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glUseProgram(m_ambientLightShader->programId);

	const auto vpInv = glm::inverse(m_activeCamera->GetViewProjectionMatrix());

	m_ambientLightShader->SendMat4("vp_inv", vpInv);
	m_ambientLightShader->SendVec3("wcs_viewPos", m_activeCamera->GetNodePositionWCS());
	m_ambientLightShader->SendVec3("ambient", m_ambient->node->GetAmbientTerm());
	m_ambientLightShader->SendTexture(m_gBuffer.depthAttachment, 0);
	m_ambientLightShader->SendCubeTexture(m_ambient->texture->id, 1);
	m_ambientLightShader->SendTexture(m_gBuffer.albedoOpacityAttachment, 2);
	m_ambientLightShader->SendTexture(m_gBuffer.emissiveAttachment, 3);
	m_ambientLightShader->SendTexture(m_gBuffer.specularAttachment, 4);

	// big triangle trick, no vao
	report_glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glDisable(GL_BLEND);
}

void GLDeferredRenderer::RenderPostProcess()
{
	// after the first post process, blend the rest on top of it
	// or copy the lightsFbo to the outFbo at the beginning of post processing

	// Dummy post process
	glViewport(0, 0, m_activeCamera->GetWidth(), m_activeCamera->GetHeight());

	// on m_outFbo
	glBindFramebuffer(GL_FRAMEBUFFER, m_outFbo);

	glUseProgram(m_dummyPostProcShader->programId);

	m_dummyPostProcShader->SendFloat("gamma", m_gamma);
	m_dummyPostProcShader->SendFloat("exposure", m_exposure);
	m_dummyPostProcShader->SendTexture(m_lightTexture, 0);

	// big triangle trick, no vao
	report_glDrawArrays(GL_TRIANGLES, 0, 3);
}

void GLDeferredRenderer::RenderWindow()
{
	auto wnd = Engine::GetMainWindow();
	glViewport(0, 0, wnd->GetWidth(), wnd->GetHeight());

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(m_windowShader->programId);

	m_windowShader->SendTexture(m_outTexture, 0);

	// big triangle trick, no vao
	report_glDrawArrays(GL_TRIANGLES, 0, 3);
}

void GLDeferredRenderer::Render()
{
	ClearBuffers();

	if (m_activeCamera) {
		// geometry pass
		RenderGBuffer();
		// light pass - blend lights on lightFbo
		RenderDirectionalLights();
		RenderSpotLights();
		RenderPunctualLights();
		// ambient pass
		RenderAmbientLight();
		RenderBoundingBoxes();
		// post process
		RenderPostProcess();
		// render to window
		RenderWindow();
	}

	// ensure writing of editor on the back buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GLEditorRenderer::Render();

	SwapBuffers();
}

void GLDeferredRenderer::RecompileShaders()
{
	m_deferredDirectionalLightShader->Reload();
	m_deferredSpotLightShader->Reload();
	m_deferredPunctualLightShader->Reload();
	m_ambientLightShader->Reload();
	m_windowShader->Reload();
	m_gBuffer.shader->Reload();
	m_dummyPostProcShader->Reload();
}

void GLDeferredRenderer::ActiveCameraResize()
{
	const auto width = m_activeCamera->GetWidth();
	const auto height = m_activeCamera->GetHeight();

	glBindTexture(GL_TEXTURE_2D, m_gBuffer.positionsAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, m_gBuffer.normalsAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, m_gBuffer.albedoOpacityAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, m_gBuffer.specularAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, m_gBuffer.emissiveAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, m_gBuffer.depthAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	glBindTexture(GL_TEXTURE_2D, m_lightTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, m_outTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
}

void GLDeferredRenderer::Update()
{
	GLRendererBase::Update();

	if (Engine::GetInput()->IsKeyPressed(Key::R)) {
		RecompileShaders();
	}

	if (Engine::GetInput()->IsKeyPressed(Key::ADD)) {
		m_gamma += 0.03f;
	}

	if (Engine::GetInput()->IsKeyPressed(Key::SUBTRACT)) {
		m_gamma -= 0.03f;
	}

	if (Engine::GetInput()->IsKeyPressed(Key::MULTIPLY)) {
		m_exposure += 0.03f;
	}

	if (Engine::GetInput()->IsKeyPressed(Key::DIVIDE)) {
		m_exposure -= 0.03f;
	}
}
} // namespace ogl
