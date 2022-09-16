#pragma once

#include <memory>
#include "CSV/TableData.h"
#include "CSV/TableImportance.h"

namespace lineweaver
{
	class Scene
	{
	public:
		Scene();
		TableData* tableData();
		TableImportance* tableImportance();

	private:
		std::unique_ptr<TableData> m_tableData;
		std::unique_ptr<TableImportance> m_tableImportance;
	};
}