#include "cDepthCubeMap.h"
#include <vector>
#include <iostream>

void RenderSceneDepth(GLuint shadowShaderProgram);
// OpenGL debug callback implementation
//void GLAPIENTRY MyDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
//	GLsizei length, const GLchar* message, const void* userParam) {
//	std::cout << "OpenGL Debug Message: " << message << std::endl;
//}

bool cDepthCubeMap::init(int width, int height, std::string& error)
{

	this->width = width;
	this->height = height;

	// Create the depth buffer
	glGenTextures(1, &m_depth);
	glBindTexture(GL_TEXTURE_2D, m_depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->width, this->height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);


	// Create the cubemap texture
	glGenTextures(1, &depthCubemapTex_ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemapTex_ID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	

	for (GLuint i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24,
			this->width, this->height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}


	glGenFramebuffers(1, &depthMapFBO_ID);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO_ID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth, 0);
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemapTex_ID, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	//glEnable(GL_DEBUG_OUTPUT);
	//glDebugMessageCallback(MyDebugCallback, nullptr);
	//glDisable(GL_DEBUG_OUTPUT);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	bool bFrameBufferIsGoodToGo = true;

	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE:
		bFrameBufferIsGoodToGo = true;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		error = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
		bFrameBufferIsGoodToGo = false;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		error = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
		bFrameBufferIsGoodToGo = false;
		break;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		error = "GL_FRAMEBUFFER_UNSUPPORTED";
		bFrameBufferIsGoodToGo = false;
		break;

	default:
		error = "Framebuffer incomplete: Unknown status " + std::to_string(status);
		bFrameBufferIsGoodToGo = false;
		break;
	}

	// If there's an OpenGL error, output the error state
	GLenum glError = glGetError();
	if (glError != GL_NO_ERROR) {
		error = "OpenGL error: " + std::to_string(glError);
		bFrameBufferIsGoodToGo = false;
	}

	// Point back to default frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return bFrameBufferIsGoodToGo;
}



bool cDepthCubeMap::shutdown(void)
{
	return false;
}

void cDepthCubeMap::renderDepthFromLightPers(GLuint shadowShaderProgram, glm::vec3 lightPos)
{


	float near_plane = 1.0f, far_plane = 1000.0f;
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, near_plane, far_plane);

	std::vector<glm::mat4> shadowTransforms = {
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)), // +X
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)), // -X
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0,  1.0,  0.0), glm::vec3(0.0,  0.0,  1.0)), // +Y
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0,  0.0), glm::vec3(0.0,  0.0, -1.0)), // -Y
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0,  0.0,  1.0), glm::vec3(0.0, -1.0,  0.0)), // +Z
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0,  0.0, -1.0), glm::vec3(0.0, -1.0,  0.0))  // -Z
	};

	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO_ID);

	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(shadowShaderProgram);
	glUniform3f(glGetUniformLocation(shadowShaderProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	glUniform1f(glGetUniformLocation(shadowShaderProgram, "far_plane"), far_plane);

	for (int i = 0; i < 6; ++i) {
		std::string name = "shadowMatrices[" + std::to_string(i) + "]";
		GLint shadowMatrices_UL = glGetUniformLocation(shadowShaderProgram, name.c_str());
		glUniformMatrix4fv(shadowMatrices_UL, 1, GL_FALSE, (const GLfloat*)&shadowTransforms[i]);
	}

	RenderSceneDepth(shadowShaderProgram); // Render scene from light's POV

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

bool cDepthCubeMap::reset(int width, int height, std::string& error)
{
	return false;
}

void cDepthCubeMap::clearDepthBuffer(void)
{
}
