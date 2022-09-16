#pragma once
#include "Renderer.h"

// currently supported rendering strategies
#include "RenderingStrategies/RenderingStrategy.h"
#include "RenderingStrategies/LinkedListRendering.h"

#include <memory>

#include <glm/glm.hpp>
#include <glbinding/gl/gl.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/Buffer.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>
#include <globjects/Framebuffer.h>
#include <globjects/Renderbuffer.h>
#include <globjects/Texture.h>
#include <globjects/base/File.h>
#include <globjects/TextureHandle.h>
#include <globjects/NamedString.h>
#include <globjects/base/StaticStringSource.h>
#include <globjects/Query.h>

namespace lineweaver
{
	class Viewer;

	class LineRenderer : public Renderer
	{
	public:
		LineRenderer(Viewer *viewer);
		virtual void display();
		virtual std::list<globjects::File*> shaderFiles() const;

	private:
		
		LinkedListRendering* renderingStrategy = NULL;

		std::unique_ptr<globjects::VertexArray> m_vao = std::make_unique<globjects::VertexArray>();
		std::unique_ptr<globjects::Buffer> m_xColumnBuffer = std::make_unique<globjects::Buffer>();
		std::unique_ptr<globjects::Buffer> m_yColumnBuffer = std::make_unique<globjects::Buffer>();
		std::unique_ptr<globjects::Buffer> m_importanceColumnBuffer = std::make_unique<globjects::Buffer>();
		
		std::unique_ptr<globjects::VertexArray> m_vaoQuad = std::make_unique<globjects::VertexArray>();
		std::unique_ptr<globjects::Buffer> m_verticesQuad = std::make_unique<globjects::Buffer>();
		
		std::unique_ptr<globjects::Program> m_programLine = std::make_unique<globjects::Program>();
		std::unique_ptr<globjects::Program> m_programBlur = std::make_unique<globjects::Program>();
		std::unique_ptr<globjects::Program> m_programBlend = std::make_unique<globjects::Program>();

		std::unique_ptr<globjects::StaticStringSource> m_shaderSourceDefines = nullptr;
		std::unique_ptr<globjects::NamedString> m_shaderDefines = nullptr;

		std::unique_ptr<globjects::File> m_shaderSourceGlobals = nullptr;
		std::unique_ptr<globjects::NamedString> m_shaderGlobals = nullptr;

		//------------------------------------------------------------------------------------------------------
		std::unique_ptr<globjects::File> m_vertexShaderSourceLine = nullptr;
		std::unique_ptr<globjects::AbstractStringSource> m_vertexShaderTemplateLine = nullptr;
		std::unique_ptr<globjects::Shader> m_vertexShaderLine = nullptr;

		std::unique_ptr<globjects::File> m_geometryShaderSourceLine = nullptr;
		std::unique_ptr<globjects::AbstractStringSource> m_geometryShaderTemplateLine = nullptr;
		std::unique_ptr<globjects::Shader> m_geometryShaderLine = nullptr;

		std::unique_ptr<globjects::File> m_fragmentShaderSourceLine = nullptr;
		std::unique_ptr<globjects::AbstractStringSource> m_fragmentShaderTemplateLine = nullptr;
		std::unique_ptr<globjects::Shader> m_fragmentShaderLine = nullptr;
		//------------------------------------------------------------------------------------------------------
		std::unique_ptr<globjects::File> m_vertexShaderSourceImage = nullptr;
		std::unique_ptr<globjects::AbstractStringSource> m_vertexShaderTemplateImage = nullptr;
		std::unique_ptr<globjects::Shader> m_vertexShaderImage = nullptr;

		std::unique_ptr<globjects::File> m_geometryShaderSourceImage = nullptr;
		std::unique_ptr<globjects::AbstractStringSource> m_geometryShaderTemplateImage = nullptr;
		std::unique_ptr<globjects::Shader> m_geometryShaderImage = nullptr;
		//------------------------------------------------------------------------------------------------------
		std::unique_ptr<globjects::File> m_fragmentShaderSourceBlur = nullptr;
		std::unique_ptr<globjects::AbstractStringSource> m_fragmentShaderTemplateBlur = nullptr;
		std::unique_ptr<globjects::Shader> m_fragmentShaderBlur = nullptr;
		//------------------------------------------------------------------------------------------------------
		std::unique_ptr<globjects::File> m_fragmentShaderSourceBlend = nullptr;
		std::unique_ptr<globjects::AbstractStringSource> m_fragmentShaderTemplateBlend = nullptr;
		std::unique_ptr<globjects::Shader> m_fragmentShaderBlend = nullptr;
		//------------------------------------------------------------------------------------------------------

		//SSBO
		std::unique_ptr<globjects::Buffer> m_intersectionBuffer = std::make_unique<globjects::Buffer>();

		//SSBO for overplotting measure implementation
		std::unique_ptr<globjects::Buffer> m_totalPixelBuffer = std::make_unique<globjects::Buffer>();
		std::unique_ptr<globjects::Buffer> m_visiblePixelBuffer = std::make_unique<globjects::Buffer>();
		

		std::unique_ptr<globjects::Texture> m_lineChartTexture = nullptr;
		std::unique_ptr<globjects::Texture> m_depthTexture = nullptr;
		std::unique_ptr<globjects::Texture> m_offsetTexture = nullptr;
		std::unique_ptr<globjects::Texture> m_blurTexture[2] = { nullptr, nullptr };

		glm::ivec2 m_framebufferSize;

		std::unique_ptr<globjects::Framebuffer> m_blurFramebuffer = nullptr;
		std::unique_ptr<globjects::Framebuffer> m_lineFramebuffer = nullptr;

		// GUI variables ----------------------------------------------------------------------------

		// supported render modes
		int m_coloringMode = 0;			// 0-None, 1-Importance, 2-Depth, 3-Random

		// allow the user to arbitrarily scale both axes
		float m_xAxisScaling = 1.0f;
		float m_yAxisScaling = 1.0f;

		// store combo ID of selected file
		std::string m_dataFilename;
		std::string m_importanceFilename;
		
		// allow highlighting a single trajectory
		bool m_enableFocusLine = false;
		int m_focusLineID = 0;

		// add support for line halos
		bool m_enableLineHalos = true;

		// Line Parameters
		float m_lineWidth = 16.0f;
		float m_smoothness = (1.0f/3.0f);		// weight used for soft depth compositing

		// provide modulation of importance
		int m_easeFunctionID = 0;

		// since overplotting measuring reduced performance, it is triggered by a button
		bool m_calculateOverplottingIndex = false;
		bool m_displayOverplottingGUI = false;

		// support focus lense feature
		bool m_enableLens = false;
		float m_lensRadius = 0.15f;

		glm::vec2 m_lensPosition;
		glm::vec2 m_delayedLensPosition;

		// support for angular brush
		bool m_enableAngularBrush = false;
		float m_brushingAngle = 0.0f;

		std::vector<unsigned int> m_totalPixelsPerTrajectory;
		std::vector<unsigned int> m_visiblePixelsPerTrajectory;
		double m_overplottingRatio = 0.0;
		// ------------------------------------------------------------------------------------------
	};

}