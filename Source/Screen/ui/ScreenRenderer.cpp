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
		shader->use();

		for (auto& s : screen->surfaces.items) drawSurface(s);

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

void ScreenRenderer::drawSurface(Surface* s)
{
	if (!s->enabled->boolValue()) return;

	Point<float> tl, tr, bl, br;

	Media* mask = s->mask->getTargetContainerAs<Media>();// dynamic_cast<Media*>(s->mask->targetContainer.get());
	std::shared_ptr<OpenGLTexture> texMask = nullptr;


	GLuint maskLocation = glGetUniformLocation(shader->getProgramID(), "mask");
	glUniform1i(maskLocation, 0);
	glActiveTexture(GL_TEXTURE0);

	if (mask != nullptr)
	{
		glBindTexture(GL_TEXTURE_2D, mask->getTextureID());
	}
	else
	{
		juce::Image whiteImage(juce::Image::PixelFormat::ARGB, 1, 1, true);
		whiteImage.setPixelAt(0, 0, Colours::white);
		texMask = std::make_shared<OpenGLTexture>();
		texMask->loadImage(whiteImage);
		texMask->bind();
	}
	glGetError();

	Media* media = s->getMedia();
	std::shared_ptr<OpenGLTexture> tex = nullptr;

	GLuint textureLocation = glGetUniformLocation(shader->getProgramID(), "tex");
	glUniform1i(textureLocation, 1);
	glActiveTexture(GL_TEXTURE1);
	glGetError();

	if (media != nullptr)
	{
		glBindTexture(GL_TEXTURE_2D, media->getTextureID());
	}


	// vertices start

	// Create a vertex buffer object
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glGetError();

	GLint posAttrib = glGetAttribLocation(shader->getProgramID(), "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), 0);
	glGetError();

	GLint surfacePosAttrib = glGetAttribLocation(shader->getProgramID(), "surfacePosition");
	glEnableVertexAttribArray(surfacePosAttrib);
	glVertexAttribPointer(surfacePosAttrib, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (void*)(2 * sizeof(float)));
	glGetError();

	GLint texAttrib = glGetAttribLocation(shader->getProgramID(), "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(4 * sizeof(float)));
	glGetError();

	GLint maskAttrib = glGetAttribLocation(shader->getProgramID(), "maskcoord");
	glEnableVertexAttribArray(maskAttrib);
	glVertexAttribPointer(maskAttrib, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(7 * sizeof(float)));
	glGetError();

	GLuint borderSoftLocation = glGetUniformLocation(shader->getProgramID(), "borderSoft");
	glUniform4f(borderSoftLocation, s->softEdgeTop->floatValue(), s->softEdgeRight->floatValue(), s->softEdgeBottom->floatValue(), s->softEdgeLeft->floatValue());
	glGetError();

	GLuint invertMaskLocation = glGetUniformLocation(shader->getProgramID(), "invertMask");
	glUniform1i(invertMaskLocation, s->invertMask->boolValue() ? 1 : 0);
	glGetError();

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glGetError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glGetError();

	s->verticesLock.enter();
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * s->vertices.size(), s->vertices.getRawDataPointer(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, s->verticesElements.size() * sizeof(GLuint), s->verticesElements.getRawDataPointer(), GL_STATIC_DRAW);
	s->verticesLock.exit();
	glGetError();

	glDrawElements(GL_TRIANGLES, s->verticesElements.size(), GL_UNSIGNED_INT, 0);
	glGetError();

	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	if (tex != nullptr) tex->unbind();

	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	if (texMask != nullptr) texMask->unbind();

	glGetError();


}


void ScreenRenderer::openGLContextClosing()
{
	glEnable(GL_BLEND);
	glDisable(GL_BLEND);
	shader = nullptr;
}


void ScreenRenderer::createAndLoadShaders()
{
	//String vertexShader, fragmentShader;

	const char* vertexShaderCode =
		"in vec2 position;\n"
		"in vec2 surfacePosition;\n"
		"in vec3 texcoord;\n"
		"in vec3 maskcoord;\n"
		"out vec3 Texcoord;\n"
		"out vec3 Maskcoord;\n"
		"out vec2 SurfacePosition;\n"
		"void main()\n"
		"{\n"
		"    Texcoord = texcoord;\n"
		"    Maskcoord = maskcoord;\n"
		"    SurfacePosition[0] = (surfacePosition[0]+1.0f)/2.0f;\n"
		"    SurfacePosition[1] = (surfacePosition[1]+1.0f)/2.0f;\n"
		"    gl_Position = vec4(position,0,1);\n"
		"}\n";

	const char* fragmentShaderCode =
		"in vec3 Texcoord;\n"
		"in vec3 Maskcoord;\n"
		"in vec2 SurfacePosition;\n"
		"out vec4 outColor;\n"
		"uniform sampler2D tex;\n"
		"uniform sampler2D mask;\n"
		"uniform vec4 borderSoft;\n"
		"uniform int invertMask;\n"
		"float map(float value, float min1, float max1, float min2, float max2) {\n"
		"return min2 + ((max2-min2)*(value-min1)/(max1-min1)); };"
		"void main()\n"
		"{\n"
		"   outColor = textureProj(tex, Texcoord);\n"
		"   vec4 maskColor = textureProj(mask, Maskcoord);\n"
		"   float alpha = 1.0f;\n"
		"   if (SurfacePosition[1] > 1-borderSoft[0])    {alpha *= map(SurfacePosition[1],1.0f,1-borderSoft[0],0.0f,1.0f);}\n" // top
		"   if (SurfacePosition[0] > 1.0f-borderSoft[1]) {alpha *= map(SurfacePosition[0], 1.0f, 1 - borderSoft[1], 0.0f, 1.0f); }\n" // right
		"   if (SurfacePosition[1] < borderSoft[2])      {alpha *= map(SurfacePosition[1],0.0f,borderSoft[2],0.0f,1.0f);}\n" // bottom
		"   if (SurfacePosition[0] < borderSoft[3])      {alpha *= map(SurfacePosition[0],0.0f,borderSoft[3],0.0f,1.0f);}\n" // left
		"	float maskValue = invertMask == 0 ? maskColor[1] : 1-maskColor[1];\n"
		"   alpha *= maskValue; \n"
		"   outColor[3] = alpha;\n"
		"}\n";

	shader.reset(new OpenGLShaderProgram(GlContextHolder::getInstance()->context));
	shader->addVertexShader(OpenGLHelpers::translateVertexShaderToV3(vertexShaderCode));
	shader->addFragmentShader(OpenGLHelpers::translateFragmentShaderToV3(fragmentShaderCode));
	shader->link();
}