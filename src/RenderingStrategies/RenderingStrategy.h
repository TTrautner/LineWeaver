#pragma once

#include "../CSV/TableData.h"
#include "../CSV/TableImportance.h"

#include <globjects/Program.h>
#include <globjects/VertexArray.h>

#include <glbinding/gl/enum.h>

namespace molumes
{
	// Base class
	class RenderingStrategy {

	protected:
		TableData* m_dataTable;
		TableImportance* m_importanceTable;

		// current CSV data vectors that are used to fill VBOs
		std::vector<float> m_activeXColumn;
		std::vector<float> m_activeYColumn;
		std::vector<float> m_activeImportance;

	public:
		RenderingStrategy(TableData* data);

		// pure virtual function providing interface framework.
		virtual void prepareDataBuffers() = 0;

		// two functions to apply importance: 1.) using a data dependent metric 2.) using a separate table
		virtual void prepareImportanceBuffer() = 0;
		virtual void prepareImportanceBuffer(TableImportance* importance) = 0;
		
		// function that is used for the actual rendering
		virtual void performRendering(globjects::Program* p, globjects::VertexArray* va) = 0;

		// access individual currently active data columns
		std::vector<float>& activeXColumn();
		std::vector<float>& activeYColumn();
		std::vector<float>& activeImportance();

		// provide functionality for sorting
		void zip(const std::vector<float> &a, const std::vector<int> &b, std::vector<std::pair<float, int>> &zipped);
		void unzip(const std::vector<std::pair<float, int>> &zipped, std::vector<float> &a, std::vector<int> &b);

		void zip(const std::vector<int> &a, const std::vector<int> &b, std::vector<std::pair<int, int>> &zipped);
		void unzip(const std::vector<std::pair<int, int>> &zipped, std::vector<int> &a, std::vector<int> &b);
	};
}