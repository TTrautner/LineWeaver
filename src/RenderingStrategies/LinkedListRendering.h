#pragma once
#include "RenderingStrategy.h"

namespace lineweaver
{
	class LinkedListRendering : public RenderingStrategy {

	public:
		LinkedListRendering(TableData* data);

		void prepareDataBuffers();

		void prepareImportanceBuffer();
		void prepareImportanceBuffer(TableImportance* importance);

		void performRendering(globjects::Program* p, globjects::VertexArray* va);
		void weaveSeries(const TableData& table);

	};
}