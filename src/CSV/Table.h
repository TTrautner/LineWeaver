#pragma once

#include <vector>
#include <glm/glm.hpp>

// header to read and write CSV files
#include "rapidcsv.h"
#include "csvfile.h"

namespace lineweaver
{
	class Table	{

	protected:
		rapidcsv::Document m_csvDocument;
		std::vector<std::string> m_columnNames;

	public:
		Table();
		virtual void load(const std::string& filePath) = 0;

		int m_numberOfTrajectories;
		int m_numberOfMaxTimesteps;

		std::vector<int> m_numberOfTimesteps;								// [trajectories]

		std::string m_fileName;
		std::string m_filePath;	

	
	};
}