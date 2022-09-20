#include "LineRenderer.h"
#include <globjects/base/File.h>
#include <globjects/State.h>
#include <iostream>
#include <filesystem>
#include <imgui.h>
#include "Viewer.h"
#include "Scene.h"
#include "CSV/TableData.h"
#include <sstream>
#include <tinyfiledialogs.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

using namespace lineweaver;
using namespace gl;
using namespace glm;
using namespace globjects;

LineRenderer::LineRenderer(Viewer* viewer) : Renderer(viewer)
{
	Shader::hintIncludeImplementation(Shader::IncludeImplementation::Fallback);

	m_intersectionBuffer->setStorage(sizeof(vec3) * 2560 * 1440 * 128 + sizeof(uint), nullptr, gl::GL_NONE_BIT);

	m_verticesQuad->setStorage(std::array<vec3, 1>({ vec3(0.0f, 0.0f, 0.0f) }), gl::GL_NONE_BIT);
	auto vertexBindingQuad = m_vaoQuad->binding(0);
	vertexBindingQuad->setBuffer(m_verticesQuad.get(), 0, sizeof(vec3));
	vertexBindingQuad->setFormat(3, GL_FLOAT);
	m_vaoQuad->enable(0);
	m_vaoQuad->unbind();

	m_shaderSourceDefines = StaticStringSource::create("");
	m_shaderDefines = NamedString::create("/defines.glsl", m_shaderSourceDefines.get());

	m_shaderSourceGlobals = File::create("./res/line/globals.glsl");
	m_shaderGlobals = NamedString::create("/globals.glsl", m_shaderSourceGlobals.get());
	//------------------------------------------------------------------------------------------------------
	m_vertexShaderSourceLine = Shader::sourceFromFile("./res/line/line-vs.glsl");
	m_geometryShaderSourceLine = Shader::sourceFromFile("./res/line/line-gs.glsl");
	m_fragmentShaderSourceLine = Shader::sourceFromFile("./res/line/line-fs.glsl");
	//------------------------------------------------------------------------------------------------------
	m_vertexShaderSourceImage = Shader::sourceFromFile("./res/line/image-vs.glsl");
	m_geometryShaderSourceImage = Shader::sourceFromFile("./res/line/image-gs.glsl");
	//------------------------------------------------------------------------------------------------------
	m_fragmentShaderSourceBlur = Shader::sourceFromFile("./res/line/blur-fs.glsl");
	//------------------------------------------------------------------------------------------------------
	m_fragmentShaderSourceBlend = Shader::sourceFromFile("./res/line/blend-fs.glsl");
	//------------------------------------------------------------------------------------------------------

	m_vertexShaderTemplateLine = Shader::applyGlobalReplacements(m_vertexShaderSourceLine.get());
	m_geometryShaderTemplateLine = Shader::applyGlobalReplacements(m_geometryShaderSourceLine.get());
	m_fragmentShaderTemplateLine = Shader::applyGlobalReplacements(m_fragmentShaderSourceLine.get());
	//------------------------------------------------------------------------------------------------------
	m_vertexShaderTemplateImage = Shader::applyGlobalReplacements(m_vertexShaderSourceImage.get());
	m_geometryShaderTemplateImage = Shader::applyGlobalReplacements(m_geometryShaderSourceImage.get());
	//------------------------------------------------------------------------------------------------------
	m_fragmentShaderTemplateBlur = Shader::applyGlobalReplacements(m_fragmentShaderSourceBlur.get());
	//------------------------------------------------------------------------------------------------------
	m_fragmentShaderTemplateBlend = Shader::applyGlobalReplacements(m_fragmentShaderSourceBlend.get());
	//------------------------------------------------------------------------------------------------------

	m_vertexShaderLine = Shader::create(GL_VERTEX_SHADER, m_vertexShaderTemplateLine.get());
	m_geometryShaderLine = Shader::create(GL_GEOMETRY_SHADER, m_geometryShaderTemplateLine.get());
	m_fragmentShaderLine = Shader::create(GL_FRAGMENT_SHADER, m_fragmentShaderTemplateLine.get());
	//------------------------------------------------------------------------------------------------------
	m_vertexShaderImage = Shader::create(GL_VERTEX_SHADER, m_vertexShaderTemplateImage.get());
	m_geometryShaderImage = Shader::create(GL_GEOMETRY_SHADER, m_geometryShaderTemplateImage.get());
	//------------------------------------------------------------------------------------------------------
	m_fragmentShaderBlur = Shader::create(GL_FRAGMENT_SHADER, m_fragmentShaderTemplateBlur.get());
	//------------------------------------------------------------------------------------------------------
	m_fragmentShaderBlend= Shader::create(GL_FRAGMENT_SHADER, m_fragmentShaderTemplateBlend.get());
	//------------------------------------------------------------------------------------------------------

	// Shader programs
	m_programLine->attach(m_vertexShaderLine.get(), m_geometryShaderLine.get(), m_fragmentShaderLine.get());
	m_programBlur->attach(m_vertexShaderImage.get(), m_geometryShaderImage.get(), m_fragmentShaderBlur.get());
	m_programBlend->attach(m_vertexShaderImage.get(), m_geometryShaderImage.get(), m_fragmentShaderBlend.get());

	m_framebufferSize = viewer->viewportSize();

	m_lineChartTexture = Texture::create(GL_TEXTURE_2D);
	m_lineChartTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_lineChartTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_lineChartTexture->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_lineChartTexture->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_lineChartTexture->image2D(0, GL_RGBA32F, m_framebufferSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	m_depthTexture = Texture::create(GL_TEXTURE_2D);
	m_depthTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_depthTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_depthTexture->setParameter(GL_TEXTURE_BORDER_COLOR, vec4(1.0,1.0,1.0,1.0));
	m_depthTexture->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	m_depthTexture->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	m_depthTexture->image2D(0, GL_DEPTH_COMPONENT, m_framebufferSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

	for (int i = 0; i < 2; i++)
	{
		m_blurTexture[i] = Texture::create(GL_TEXTURE_2D);
		m_blurTexture[i]->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		m_blurTexture[i]->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		m_blurTexture[i]->setParameter(GL_TEXTURE_BORDER_COLOR, vec4(1.0, 1.0, 1.0, 1.0));
		m_blurTexture[i]->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		m_blurTexture[i]->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		m_blurTexture[i]->image2D(0, GL_RGBA, m_framebufferSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	}

	m_offsetTexture = Texture::create(GL_TEXTURE_2D);
	m_offsetTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_offsetTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_offsetTexture->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_offsetTexture->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_offsetTexture->image2D(0, GL_R32UI, m_framebufferSize, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);

	m_blurFramebuffer = Framebuffer::create();
	m_blurFramebuffer->attachTexture(GL_COLOR_ATTACHMENT0, m_blurTexture[0].get());
	m_blurFramebuffer->attachTexture(GL_COLOR_ATTACHMENT1, m_blurTexture[1].get());
	m_blurFramebuffer->attachTexture(GL_DEPTH_ATTACHMENT, m_depthTexture.get());
	m_blurFramebuffer->setDrawBuffers({ GL_COLOR_ATTACHMENT0 });

	m_lineFramebuffer = Framebuffer::create();
	m_lineFramebuffer->attachTexture(GL_COLOR_ATTACHMENT0, m_lineChartTexture.get());
	m_lineFramebuffer->attachTexture(GL_DEPTH_ATTACHMENT, m_depthTexture.get());
	m_lineFramebuffer->setDrawBuffers({ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1});
}

std::list<globjects::File*> LineRenderer::shaderFiles() const
{
	return std::list<globjects::File*>({
		m_shaderSourceGlobals.get(),

		m_vertexShaderSourceLine.get(),
		m_geometryShaderSourceLine.get(),
		m_fragmentShaderSourceLine.get(),

		m_vertexShaderSourceImage.get(),
		m_geometryShaderSourceImage.get(),

		m_fragmentShaderSourceBlur.get(),
		m_fragmentShaderSourceBlend.get()
		});
}

void LineRenderer::display()
{
	auto currentState = State::currentState();

	if (viewer()->viewportSize() != m_framebufferSize)
	{
		m_framebufferSize = viewer()->viewportSize();
		
		m_lineChartTexture->image2D(0, GL_RGBA32F, m_framebufferSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		m_depthTexture->image2D(0, GL_DEPTH_COMPONENT, m_framebufferSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
		m_offsetTexture->image2D(0, GL_R32UI, m_framebufferSize, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);

		for (int i=0;i<2;i++)
			m_blurTexture[i]->image2D(0, GL_RGBA, m_framebufferSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	}

	// retrieve/compute all necessary matrices and related properties
	const mat4 viewMatrix = viewer()->viewTransform();
	const mat4 inverseViewMatrix = inverse(viewMatrix);
	const mat4 modelViewMatrix = viewer()->modelViewTransform();
	const mat4 inverseModelViewMatrix = inverse(modelViewMatrix);
	const mat4 modelLightMatrix = viewer()->modelLightTransform();
	const mat4 inverseModelLightMatrix = inverse(modelLightMatrix);
	const mat4 modelViewProjectionMatrix = viewer()->modelViewProjectionTransform();
	const mat4 inverseModelViewProjectionMatrix = inverse(modelViewProjectionMatrix);
	const mat4 projectionMatrix = viewer()->projectionTransform();
	const mat4 inverseProjectionMatrix = inverse(projectionMatrix);
	const mat3 normalMatrix = mat3(transpose(inverseModelViewMatrix));
	const mat3 inverseNormalMatrix = inverse(normalMatrix);
	const ivec2 viewportSize = viewer()->viewportSize();

	vec4 worldLightPosition = inverseModelLightMatrix * vec4(0.0f, 0.0f, 0.0f, 1.0f);

	double mouseX, mouseY;
	glfwGetCursorPos(viewer()->window(), &mouseX, &mouseY);
	m_lensPosition = vec2(2.0f*float(mouseX) / float(viewportSize.x) - 1.0f, -2.0f*float(mouseY) / float(viewportSize.y) + 1.0f);

	float fT = 0.5f;
	m_delayedLensPosition = m_delayedLensPosition * (1.0f - fT) + m_lensPosition * fT;

	vec4 projectionInfo(float(-2.0 / (viewportSize.x * projectionMatrix[0][0])),
		float(-2.0 / (viewportSize.y * projectionMatrix[1][1])),
		float((1.0 - (double)projectionMatrix[0][2]) / projectionMatrix[0][0]),
		float((1.0 + (double)projectionMatrix[1][2]) / projectionMatrix[1][1]));

	float projectionScale = float(viewportSize.y) / fabs(2.0f / projectionMatrix[1][1]);

	vec4 nearPlane = inverseProjectionMatrix * vec4(0.0, 0.0, -1.0, 1.0);
	nearPlane /= nearPlane.w;

	// boolean variables used to automatically update data and importance
	static bool dataChanged = false;
	static bool importanceChanged = false;

	// Scatterplot GUI --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	ImGui::Begin("Importance Driven Dense Line Graphs");

	if (ImGui::CollapsingHeader("CSV-Files", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static int fileMode = 0;
		ImGui::Combo("File Mode", &fileMode, "Trajectory\0Series\0");

		std::string oldDataFilename = m_dataFilename;

		if (ImGui::Button("Browse##1"))
		{
			const char* filterExtensions[] = { "*.csv" };
			const char* openfileName = tinyfd_openFileDialog("Open Data File", "./", 1, filterExtensions, "CSV Files (*.csv)", 0);

			if (openfileName)
				m_dataFilename = std::string(openfileName);
		}

		ImGui::SameLine();
		ImGui::InputTextWithHint("Data File", "Press button to load new file", (char*)m_dataFilename.c_str(), m_dataFilename.size(), ImGuiInputTextFlags_ReadOnly);

		if (m_dataFilename != oldDataFilename || viewer()->enforcedDataRefresh())
		{
			//std::cout << "File selection event - " << "File: " << m_fileDataID << "\n";
			// initialize data able
			if (fileMode == 0)
				viewer()->scene()->tableData()->load(m_dataFilename);
			else
			{	
				if (m_dataFilename.find("AndrewsPlot") != std::string::npos ) {

					// if data set is an andrews plot (currently indicated by name) 
					// first, load data without duplicating first and last entry
					viewer()->scene()->tableData()->loadAndrewsSeries(m_dataFilename);

					// second, perform andrews transformation
					viewer()->scene()->tableData()->andrewsTransform(96);

				} else {

					// otherwise, load series duplicating first and last element as usual
					viewer()->scene()->tableData()->loadSeries(m_dataFilename);
				}
			}

			// assign default rendering strategy 
			renderingStrategy = new LinkedListRendering(viewer()->scene()->tableData());

			// load data and apply implicite importance metric
			renderingStrategy->prepareDataBuffers();

			if (fileMode == 0)
				renderingStrategy->prepareImportanceBuffer();
			else
				renderingStrategy->weaveSeries(*viewer()->scene()->tableData());


			if(renderingStrategy != nullptr)
			{
				// reset importance to default if file was updated otherwise keep it
				if(m_dataFilename != oldDataFilename)
					importanceChanged = true;

				// update status
				dataChanged = true;
				m_displayOverplottingGUI = false;
			}
		}

		std::string oldImportanceFilename = m_importanceFilename;

		if (ImGui::Button("Browse##2"))
		{
			const char* filterExtensions[] = { "*.csv" };
			const char* openfileName = tinyfd_openFileDialog("Open Data File", "./", 1, filterExtensions, "CSV Files (*.csv)", 0);

			if (openfileName)
				m_importanceFilename = std::string(openfileName);
		}

		ImGui::SameLine();
		ImGui::InputTextWithHint("Importance File", "Press button to load new file", (char*)m_importanceFilename.c_str(), m_importanceFilename.size(), ImGuiInputTextFlags_ReadOnly);

		if ((oldImportanceFilename != m_importanceFilename || viewer()->enforcedImportanceRefresh()) && renderingStrategy != NULL)
		{
			// initialize importance table
			viewer()->scene()->tableImportance()->load(m_importanceFilename);

			// load external importance data
			renderingStrategy->prepareImportanceBuffer(viewer()->scene()->tableImportance());

			importanceChanged = true;
			m_displayOverplottingGUI = false;
		}

		if (dataChanged)
		{
			// update VBOs for both columns
			m_xColumnBuffer->setData(renderingStrategy->activeXColumn(), GL_STATIC_DRAW);
			m_yColumnBuffer->setData(renderingStrategy->activeYColumn(), GL_STATIC_DRAW);

			auto vertexBinding = m_vao->binding(0);
			vertexBinding->setAttribute(0);
			vertexBinding->setBuffer(m_xColumnBuffer.get(), 0, sizeof(float));
			vertexBinding->setFormat(1, GL_FLOAT);
			m_vao->enable(0);

			vertexBinding = m_vao->binding(1);
			vertexBinding->setAttribute(1);
			vertexBinding->setBuffer(m_yColumnBuffer.get(), 0, sizeof(float));
			vertexBinding->setFormat(1, GL_FLOAT);
			m_vao->enable(1);

			// Scaling the model's bounding box to the canonical view volume
			vec3 boundingBoxSize = viewer()->scene()->tableData()->maximumBounds() - viewer()->scene()->tableData()->minimumBounds();
			float maximumSize = std::max({ boundingBoxSize.x, boundingBoxSize.y, boundingBoxSize.z });
			mat4 modelTransform = scale(vec3(2.0f) / vec3(1.25f*boundingBoxSize.x, 1.25f*boundingBoxSize.y,1.0));
			modelTransform = modelTransform * translate(-0.5f*(viewer()->scene()->tableData()->minimumBounds() + viewer()->scene()->tableData()->maximumBounds()));
			viewer()->setModelTransform(modelTransform);

			// store diameter of current line chart and initialize light position
			viewer()->m_lineChartDiameter = sqrt(pow(boundingBoxSize.x, 2) + pow(boundingBoxSize.y, 2));

			// initial position of the light source (azimuth 120 degrees, elevation 45 degrees, 5 times the distance to the object in center) ---------------------------------------------------------------------------------------------------------
			glm::mat4 viewTransform = viewer()->viewTransform();
			glm::vec3 initLightDir = normalize(glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(120.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
			glm::mat4 newLightTransform = glm::inverse(viewTransform)*glm::translate(mat4(1.0f), (5 * viewer()->m_lineChartDiameter*initLightDir))*viewTransform;
			viewer()->setLightTransform(newLightTransform);
			//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

			// reset values
			m_focusLineID = 0;

			// update status
			dataChanged = false;
		}

		if (importanceChanged)
		{
			if (renderingStrategy->activeImportance().empty() == false) {
				m_importanceColumnBuffer->setData(renderingStrategy->activeImportance(), GL_STATIC_DRAW);

				auto vertexBinding = m_vao->binding(2);
				vertexBinding->setAttribute(2);
				vertexBinding->setBuffer(m_importanceColumnBuffer.get(), 0, sizeof(float));
				vertexBinding->setFormat(1, GL_FLOAT);
				m_vao->enable(2);
			}

			// update status
			importanceChanged = false;
		}

		ImGui::Combo("Ease Function", &m_easeFunctionID, "Linear\0In Sine\0Out Sine\0In Out Sine\0In Quad\0Out Quad\0In Out Quad\0In Cubic\0Out Cubic\0In Out Cubic\0In Quart\0Out Quart\0In Out Quart\0In Quint\0Out Quint\0In Out Quitn\0In Expo\0Out Expo\0In Out Expo\0");
	}

	// allow the user to arbitrarily scale both axes
	ImGui::SliderFloat("x-Axis Scale", &m_xAxisScaling, 0.1f, 10.0f);
	ImGui::SliderFloat("y-Axis Scale", &m_yAxisScaling, 0.1f, 10.0f);

	if (ImGui::Button("Reset"))
	{
		m_xAxisScaling = 1.0f;
		m_yAxisScaling = 1.0f;
	}

	if (ImGui::CollapsingHeader("Line Properties", ImGuiTreeNodeFlags_DefaultOpen))
	{

		ImGui::Combo("Color Mode", &m_coloringMode, "None\0Importance\0Depth\0Random\0");
		ImGui::SliderFloat("Line Width", &m_lineWidth, 1.0f, 128.0f);
		ImGui::SliderFloat("Smoothness", &m_smoothness, 0.0f, 1.0f);
		
		ImGui::Checkbox("Enable Line-Halos", &m_enableLineHalos);

		ImGui::Checkbox("Enable Focus-Line", &m_enableFocusLine);
		ImGui::SliderInt("Focus-Line", &m_focusLineID, 0, viewer()->scene()->tableData()->m_numberOfTrajectories - 1);
	}

	if (ImGui::CollapsingHeader("Lens Feature", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::SliderFloat("Lens Radius", &m_lensRadius, 0.0f, 1.0f);

		ImGui::Checkbox("Enable Focus-Lens", &m_enableLens);
		ImGui::Checkbox("Enable Angular-Brushing", &m_enableAngularBrush);

		ImGui::SliderFloat("Brushing Angle", &viewer()->m_scrollWheelAngle, -90.0f, 90.0f);
		m_brushingAngle = viewer()->m_scrollWheelAngle;
	}

	if (ImGui::CollapsingHeader("Overplotting Measurement", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::Button("Compute") || viewer()->enforcedOverplottingComp())
		{
			m_calculateOverplottingIndex = true;
			m_displayOverplottingGUI = false;
		}

		if(m_displayOverplottingGUI){

			// print result
			ImGui::Text("Overplotting index = %f", m_overplottingRatio);

			// print individual sub-results
			for (int i = 0; i < viewer()->scene()->tableData()->m_numberOfTrajectories; i++) {
				ImGui::Text("Trajectory %i = %u out of %u", i, m_visiblePixelsPerTrajectory.at(i), m_totalPixelsPerTrajectory.at(i));
			}
		}
	}

	ImGui::End();

	// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	int numberOfTrajectories = viewer()->scene()->tableData()->m_numberOfTrajectories;
	std::vector<int> numberOfTimesteps = viewer()->scene()->tableData()->m_numberOfTimesteps;

	// do not render if either the dataset was not loaded or the window is minimized 
	if (renderingStrategy == NULL || viewer()->viewportSize().x == 0 || viewer()->viewportSize().y == 0) {
		return;
	}

	std::string defines = "";

	if (m_coloringMode == 1)
		defines += "#define IMPORTANCE_AS_OPACITY\n";
	else if (m_coloringMode == 2)
		defines += "#define DEPTH_LUMINANCE_COLOR\n";
	else if(m_coloringMode == 3)
		defines += "#define RANDOM_LINE_COLORS\n";

	if (m_enableFocusLine)
		defines += "#define FOCUS_LINE\n";

	if(m_enableLineHalos)
		defines += "#define LINE_HALOS\n";

	if (LinkedListRendering* r = dynamic_cast<LinkedListRendering*>(renderingStrategy))
		defines += "#define RS_LINKEDLIST\n";

	if (m_calculateOverplottingIndex)
		defines += "#define CALCULATE_OVERPLOTTING_INDEX\n";

	if (m_enableLens)
		defines += "#define LENS_FEATURE\n";

	if (m_enableAngularBrush)
		defines += "#define ANGULAR_BRUSHING\n";

	if (m_easeFunctionID == 0)
		defines += "#define EASE_LINEAR\n";
	else if (m_easeFunctionID == 1)
		defines += "#define EASE_IN_SINE\n";
	else if (m_easeFunctionID == 2)
		defines += "#define EASE_OUT_SINE\n";
	else if (m_easeFunctionID == 3)
		defines += "#define EASE_IN_OUT_SINE\n";
	else if (m_easeFunctionID == 4)
		defines += "#define EASE_IN_QUAD\n";
	else if (m_easeFunctionID == 5)
		defines += "#define EASE_OUT_QUAD\n";
	else if (m_easeFunctionID == 6)
		defines += "#define EASE_IN_OUT_QUAD\n";
	else if (m_easeFunctionID == 7)
		defines += "#define EASE_IN_CUBIC\n";
	else if (m_easeFunctionID == 8)
		defines += "#define EASE_OUT_CUBIC\n";
	else if (m_easeFunctionID == 9)
		defines += "#define EASE_IN_OUT_CUBIC\n";
	else if (m_easeFunctionID == 10)
		defines += "#define EASE_IN_QUART\n";
	else if (m_easeFunctionID == 11)
		defines += "#define EASE_OUT_QUART\n";
	else if (m_easeFunctionID == 12)
		defines += "#define EASE_IN_OUT_QUART\n";
	else if (m_easeFunctionID == 13)
		defines += "#define EASE_IN_QUINT\n";
	else if (m_easeFunctionID == 14)
		defines += "#define EASE_OUT_QUINT\n";
	else if (m_easeFunctionID == 15)
		defines += "#define EASE_IN_OUT_QUINT\n";
	else if (m_easeFunctionID == 16)
		defines += "#define EASE_IN_EXPO\n";
	else if (m_easeFunctionID == 17)
		defines += "#define EASE_OUT_EXPO\n";
	else if (m_easeFunctionID == 18)
		defines += "#define EASE_IN_OUT_EXPO\n";

	if (defines != m_shaderSourceDefines->string())
	{
		m_shaderSourceDefines->setString(defines);

		for (auto& s : shaderFiles())
			s->reload();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Line rendering pass and linked list generation
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	m_lineFramebuffer->bind();

	glClearDepth(1.0f);
	glClearColor(viewer()->backgroundColor().r, viewer()->backgroundColor().g, viewer()->backgroundColor().b, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	// test different blending options interactively: --------------------------------------------------
	// https://andersriggelsen.dk/glblendfunc.php

	// allow blending for the classical line chart color-attachment (0) of the line frame-buffer
	/*
	glEnablei(GL_BLEND, 0);
	glBlendFunci(0, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquationi(0, GL_FUNC_ADD);
	*/
	
	// SSBO --------------------------------------------------------------------------------------------
	m_intersectionBuffer->bindBase(GL_SHADER_STORAGE_BUFFER, 1);
	
	const uint intersectionClearValue = 1;
	m_intersectionBuffer->clearSubData(GL_R32UI, 0, sizeof(uint), GL_RED_INTEGER, GL_UNSIGNED_INT, &intersectionClearValue);

	const uint offsetClearValue = 0;
	m_offsetTexture->clearImage(0, GL_RED_INTEGER, GL_UNSIGNED_INT, &offsetClearValue);
	// -------------------------------------------------------------------------------------------------

	m_offsetTexture->bindImageTexture(0, 0, false, 0, GL_READ_WRITE, GL_R32UI);

	mat4 modelTransform = viewer()->modelTransform();
	mat4 inverseModelTransform = inverse(modelTransform);
	float scaledLineWidth = length(vec2(inverseModelViewProjectionMatrix * vec4(0.0f, 0.0f, 0.0f, 1.0f) - inverseModelViewProjectionMatrix *(vec4(m_lineWidth, 0.0f, 0.0f, 1.0f)))/ vec2(viewportSize) );

	m_programLine->setUniform("modelViewProjectionMatrix", modelViewProjectionMatrix);

	m_programLine->setUniform("xAxisScaling", m_xAxisScaling);
	m_programLine->setUniform("yAxisScaling", m_yAxisScaling);

	m_programLine->setUniform("lineWidth", scaledLineWidth);
	m_programLine->setUniform("lineColor", viewer()->lineChartColor());

	m_programLine->setUniform("viewportSize", vec2(viewportSize));

	m_programLine->setUniform("haloColor", viewer()->haloColor());
	m_programLine->setUniform("focusLineColor", viewer()->focusLineColor());

	if (m_enableFocusLine) {
		m_programLine->setUniform("focusLineID", m_focusLineID);
	} else {
		m_programLine->setUniform("focusLineID", -1);
	}

	m_programLine->setUniform("lensPosition", m_lensPosition);
	m_programLine->setUniform("delayedLensPosition", m_delayedLensPosition);
	m_programLine->setUniform("lensRadius", m_lensRadius);
	
	m_programLine->setUniform("brushingAngle", m_brushingAngle);

	m_vao->bind();
	m_programLine->use();

	renderingStrategy->performRendering(m_programLine.get(), m_vao.get());

	m_programLine->release();
	m_vao->unbind();

	//m_intersectionBuffer->unbind(GL_SHADER_STORAGE_BUFFER);

	m_offsetTexture->unbindImageTexture(0);

	//m_lineFramebuffer->unbind();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// disable blending for draw buffer 0 (line graph texture)
	//glDisablei(GL_BLEND, 0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Blending dependent on current rendering strategy
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// make sure lines are drawn on top of each other
	//glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	m_blurFramebuffer->bind();
	glDepthMask(GL_FALSE);		


	m_vaoQuad->bind();
	m_programBlur->use();
	m_programBlur->setUniform("blurTexture", 0);

	m_depthTexture->bindActive(0);

	const int blutIterations = 3;
	float blurOffset = 8.0;

	int blurIndex = 0;

	for (int i = 0; i < blutIterations; i++)
	{
		m_programBlur->setUniform("offset", blurOffset);
		m_blurFramebuffer->setDrawBuffers({ GL_COLOR_ATTACHMENT0 + (1 - blurIndex) });
		m_vaoQuad->drawArrays(GL_POINTS, 0, 1);

		blurIndex = 1 - blurIndex;
		blurOffset = max(1.0f, 0.5f * blurOffset);

		m_blurTexture[blurIndex]->bindActive(0);
	}

	m_programBlur->release();
	m_vaoQuad->unbind();

	glDepthMask(GL_TRUE);
	m_blurFramebuffer->unbind();
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	m_lineFramebuffer->bind();

	// SSBO --------------------------------------------------------------------------------------------
	m_intersectionBuffer->bindBase(GL_SHADER_STORAGE_BUFFER, 1);

	if (m_calculateOverplottingIndex) {

		// reset buffers in case they were already initialized
		m_totalPixelBuffer = std::make_unique<globjects::Buffer>();
		m_visiblePixelBuffer = std::make_unique<globjects::Buffer>();

		// initialize SSAOs
		m_totalPixelBuffer->setStorage(sizeof(uint) * numberOfTrajectories, nullptr, gl::GL_NONE_BIT);
		m_visiblePixelBuffer->setStorage(sizeof(uint) * numberOfTrajectories, nullptr, gl::GL_NONE_BIT);

		// clear and prepare buffers
		m_totalPixelBuffer->bindBase(GL_SHADER_STORAGE_BUFFER, 2);
		m_visiblePixelBuffer->bindBase(GL_SHADER_STORAGE_BUFFER, 3);

		const uint bufferCounterClearValue = 0;
		m_totalPixelBuffer->clearSubData(GL_R32UI, 0, sizeof(uint) * numberOfTrajectories, GL_RED_INTEGER, GL_UNSIGNED_INT, &bufferCounterClearValue);
		m_visiblePixelBuffer->clearSubData(GL_R32UI, 0, sizeof(uint) * numberOfTrajectories, GL_RED_INTEGER, GL_UNSIGNED_INT, &bufferCounterClearValue);
	}
	// -------------------------------------------------------------------------------------------------

	m_offsetTexture->bindActive(0);
	m_blurTexture[blurIndex]->bindActive(1);

	m_programBlend->setUniform("offsetTexture", 0);
	m_programBlend->setUniform("blurTexture", 1);

	m_programBlend->setUniform("backgroundColor", viewer()->backgroundColor());
	m_programBlend->setUniform("smoothness", m_smoothness);

	m_programBlend->setUniform("viewportSize", vec2(viewportSize));
	m_programBlend->setUniform("lensPosition", m_lensPosition);
	m_programBlend->setUniform("lensRadius", m_lensRadius);
	m_programBlend->setUniform("lensBorderColor", viewer()->lensColor());

	m_vaoQuad->bind();
	
	m_programBlend->use();
	m_vaoQuad->drawArrays(GL_POINTS, 0, 1);
	m_programBlend->release();
	
	m_vaoQuad->unbind();
	
	// SSBO --------------------------------------------------------------------------------------------
	m_intersectionBuffer->unbind(GL_SHADER_STORAGE_BUFFER);

	if (m_calculateOverplottingIndex) {

		// force GPU to finish work before we start reading
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		//		     # visible pixels (per trajectory)
		//	    Sum ------------------------------------
		//		     # total pixels   (per trajectory)
		// 1 -	-------------------------------------------
		//		     # number of trajectories (total)

		m_overplottingRatio = 0;
		m_totalPixelsPerTrajectory.clear();
		m_visiblePixelsPerTrajectory.clear();

		for (int i = 0; i < numberOfTrajectories; i++) {
			
			uint currentTotalPixels = 0;
			m_totalPixelBuffer->getSubData(i * sizeof(uint), sizeof(uint), &currentTotalPixels);
			m_totalPixelsPerTrajectory.push_back(currentTotalPixels);
			
			uint currentVisiblePixels = 0;
			m_visiblePixelBuffer->getSubData(i * sizeof(uint), sizeof(uint), &currentVisiblePixels);
			m_visiblePixelsPerTrajectory.push_back(currentVisiblePixels);

			m_overplottingRatio += (double)currentVisiblePixels/ (double)currentTotalPixels;
		}

		m_overplottingRatio = 1-(m_overplottingRatio/numberOfTrajectories);

		// release bound SSBO
		m_totalPixelBuffer->unbind(GL_SHADER_STORAGE_BUFFER);
		m_visiblePixelBuffer->unbind(GL_SHADER_STORAGE_BUFFER);

		m_calculateOverplottingIndex = false;
		
		// display results in GUI
		m_displayOverplottingGUI = true;
	}
	// -------------------------------------------------------------------------------------------------

	m_blurTexture[blurIndex]->unbindActive(1);
	m_offsetTexture->unbindActive(0);

	m_lineFramebuffer->unbind();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// Blit final image into visible framebuffer
	m_lineFramebuffer->blit(GL_COLOR_ATTACHMENT0, { 0,0,viewer()->viewportSize().x, viewer()->viewportSize().y }, Framebuffer::defaultFBO().get(), GL_BACK, { 0,0,viewer()->viewportSize().x, viewer()->viewportSize().y }, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	// Restore OpenGL state
	currentState->apply();
}