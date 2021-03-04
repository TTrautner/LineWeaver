#pragma once

#include "Table.h"
#include "TableData.h"

namespace molumes
{
	class TableImportance : public Table {

	public:
		TableImportance();
		void load(const std::string& filePath);

		std::vector<std::vector<float>> m_importance;					// [trajectories,timesteps]
	};
}