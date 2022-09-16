#pragma once

#include "TableImportance.h"

#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <iterator>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <array>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <globjects/globjects.h>
#include <globjects/logging.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

using namespace lineweaver;
using namespace glm;

TableImportance::TableImportance() : Table()
{

}

void TableImportance::load(const std::string& filePath)
{

	m_filePath = filePath;

	// extract file name ---------------------------------------------------------
	std::string tempPath = m_filePath.substr(6);
	std::replace(tempPath.begin(), tempPath.end(), '/', ' ');

	std::vector<std::string> stringArray;
	std::stringstream ss(tempPath);
	std::string temp;
	while (ss >> temp)
		stringArray.push_back(temp);

	m_fileName = stringArray.at(stringArray.size() - 1);
	//----------------------------------------------------------------------------

	rapidcsv::Document loadedDocument(m_filePath, rapidcsv::LabelParams(0, -1));
	m_csvDocument = loadedDocument;

	m_numberOfTrajectories = m_csvDocument.GetRowCount();
	m_numberOfMaxTimesteps = -1;

	// get titles of all columns
	m_columnNames = m_csvDocument.GetColumnNames();
	int maxTimeSteps = m_csvDocument.GetColumnCount();


	// clear buffers if they already contain data ----------
	m_importance.clear();

	m_numberOfTimesteps.clear();
	//------------------------------------------------------

	// iterate over trajectories
	for (int y = 0; y < m_numberOfTrajectories; y++) {

		std::vector<std::string> currentTrajectory = m_csvDocument.GetRow<std::string>(y);
		std::vector<float> importanceTemp;

		int cellCount = 0;
		float importanceValue;

		// iterate over time-steps
		for (int x = 0; x < maxTimeSteps; x++) {

			//check if cell contains valid values
			std::string importanceString = currentTrajectory.at(x);

			if (importanceString != "") {
				cellCount++;

				importanceValue = stof(importanceString);

				// convert string into float values
				importanceTemp.push_back(importanceValue);
			}

			// GL_LINE_STRIP_ADJACENCY requires the first and last element to be duplicated ------------
			if (x == 0) {

				importanceTemp.push_back(importanceValue);

				cellCount++;
			}
		}

		// duplicate last item
		importanceTemp.push_back(importanceValue);

		cellCount++;
		//----------------------------------------------------------------------------------------------

		// update vector that stores the number of valid time-steps per trajectory
		m_numberOfTimesteps.push_back(cellCount);

		// find out how long the longest trajectory is
		if (cellCount > m_numberOfMaxTimesteps) {
			m_numberOfMaxTimesteps = cellCount;
		}

		// update trajectory vectors
		m_importance.push_back(importanceTemp);

		importanceTemp.clear();
	}
}
