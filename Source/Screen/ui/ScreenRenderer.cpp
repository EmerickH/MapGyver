/*
  ==============================================================================

	ScreenRenderer.cpp
	Created: 19 Nov 2023 11:44:37am
	Author:  bkupe

  ==============================================================================
*/

#include "Screen/ScreenIncludes.h"
#include "Common/CommonIncludes.h"
#include "Media/MediaIncludes.h"

using namespace juce::gl;

ScreenRenderer::ScreenRenderer(Screen* screen) :
	screen(screen)
{
	GlContextHolder::getInstance()->registerOpenGlRenderer(this);
}

ScreenRenderer::~ScreenRenderer()
{
	if (GlContextHolder::getInstanceWithoutCreating() != nullptr) GlContextHolder::getInstance()->unregisterOpenGlRenderer(this);
}

void ScreenRenderer::newOpenGLContextCreated()
{
	// Set up your OpenGL state here
	createAndLoadShaders();
	frameBuffer.initialise(GlContextHolder::getInstance()->context, screen->screenWidth->intValue(), screen->screenHeight->intValue());
}

void ScreenRenderer::renderOpenGL()
{

	// D�finir la vue OpenGL en fonction de la taille du composant

	frameBuffer.makeCurrentRenderingTarget();
	glClearColor(0, 0, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	Init2DViewport(frameBuffer.getWidth(), frameBuffer.getHeight());

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	if (shader != nullptr)
	{

		for (auto& s : screen->surfaces.items) 
		{
			shader->use();
			GLuint shaderProgram = shader->getProgramID();
			if (s->showTestPattern->boolValue()) {
				shaderProgram = shaderTest->getProgramID();
				shaderTest->use();
			}
			s->draw(shaderProgram);
		}

		glUseProgram(0);
		glGetError();
	}

	//for testing flipping
	//glBegin(GL_QUADS);
	//glColor3f(1, 1, 1);
	//glTexCoord2f(0, 1); glVertex2f(0, 0);
	//glTexCoord2f(0, 0); glVertex2f(0, 50);
	//glTexCoord2f(1, 0); glVertex2f(50, 50);
	//glTexCoord2f(1, 1); glVertex2f(50, 0);
	//glEnd();

	frameBuffer.releaseAsRenderingTarget();

}

void ScreenRenderer::openGLContextClosing()
{
	glEnable(GL_BLEND);
	glDisable(GL_BLEND);
	shader = nullptr;
	shaderTest = nullptr;
}


void ScreenRenderer::createAndLoadShaders()
{
	shader.reset(new OpenGLShaderProgram(GlContextHolder::getInstance()->context));
	shader->addVertexShader(OpenGLHelpers::translateVertexShaderToV3(BinaryData::VertexShaderMainSurface_glsl));
	shader->addFragmentShader(OpenGLHelpers::translateFragmentShaderToV3(BinaryData::fragmentShaderMainSurface_glsl));
	shader->link();

	File fileToRead("C:\\repos\\RuleMaPool\\Resources\\fragmentShaderTestGrid.glsl");
	FileInputStream inputStream(fileToRead);
	String fileContent = inputStream.readString();

	shaderTest.reset(new OpenGLShaderProgram(GlContextHolder::getInstance()->context));
	shaderTest->addVertexShader(OpenGLHelpers::translateVertexShaderToV3(BinaryData::VertexShaderMainSurface_glsl));
	//shaderTest->addFragmentShader(OpenGLHelpers::translateFragmentShaderToV3(BinaryData::fragmentShaderTestGrid_glsl));
	shaderTest->addFragmentShader(OpenGLHelpers::translateFragmentShaderToV3(fileContent));
	shaderTest->link();

}