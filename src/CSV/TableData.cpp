#pragma once

#include "TableData.h"

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
#include <glm/gtc/constants.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

using namespace molumes;
using namespace glm;

TableData::TableData() : Table()
{

}

void TableData::load(const std::string& filePath)
{

	m_filePath = filePath;

	// extract file name ---------------------------------------------------------
	std::string tempPath = m_filePath;
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
	m_XTrajectories.clear();
	m_YTrajectories.clear();

	m_numberOfTimesteps.clear();
	//------------------------------------------------------

	// iterate over trajectories
	for (int y = 0; y < m_numberOfTrajectories; y++) {

		std::vector<std::string> currentTrajectory = m_csvDocument.GetRow<std::string>(y);

		int cellCount = 0;
		float xValue, yValue;

		std::vector<float> xTemp, yTemp;

		// iterate over time-steps
		for (int x = 0; x < maxTimeSteps; x++) {

			//check if cell contains valid values
			std::string currentTimeStep = currentTrajectory.at(x);


			if (currentTimeStep != "") {
				cellCount++;

				std::string xString = currentTimeStep.substr(2, currentTimeStep.find(',') - 2);
				std::string yString = currentTimeStep.substr(currentTimeStep.find(',') + 1, currentTimeStep.length() - currentTimeStep.find(',') - 3);

				// convert string into float values
				xValue = stof(xString);
				yValue = stof(yString);

				if (x == 0 && y == 0) {
					//initialize bounding box
					m_maximumBounds.x = m_minimumBounds.x = xValue;
					m_maximumBounds.y = m_minimumBounds.y = yValue;
				}
				else {
					// update bounding volume depending on X and Y values
					m_minimumBounds.x = min(m_minimumBounds.x, xValue);
					m_maximumBounds.x = max(m_maximumBounds.x, xValue);

					m_minimumBounds.y = min(m_minimumBounds.y, yValue);
					m_maximumBounds.y = max(m_maximumBounds.y, yValue);
				}

				xTemp.push_back(xValue);
				yTemp.push_back(yValue);
			}

			// GL_LINE_STRIP_ADJACENCY requires the first and last element to be duplicated ------------
			if (x == 0) {

				xTemp.push_back(xValue);
				yTemp.push_back(yValue);

				cellCount++;
			}
		}

		// duplicate last item
		xTemp.push_back(xValue);
		yTemp.push_back(yValue);

		cellCount++;
		//----------------------------------------------------------------------------------------------

		// update vector that stores the number of valid time-steps per trajectory
		m_numberOfTimesteps.push_back(cellCount);

		// find out how long the longest trajectory is
		if (cellCount > m_numberOfMaxTimesteps) {
			m_numberOfMaxTimesteps = cellCount;
		}

		// update trajectory vectors
		m_XTrajectories.push_back(xTemp);
		m_YTrajectories.push_back(yTemp);

		xTemp.clear();
		yTemp.clear();
	}
}

void TableData::loadSeries(const std::string& filePath)
{

	m_filePath = filePath;

	// extract file name ---------------------------------------------------------
	std::string tempPath = m_filePath;
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
	int maxTimeSteps = m_csvDocument.GetColumnCount()-1;


	// clear buffers if they already contain data ----------
	m_XTrajectories.clear();
	m_YTrajectories.clear();
	m_numberOfTimesteps.clear();
	m_clusterIds.clear();
	//------------------------------------------------------

	std::unordered_map < std::string, int> clusterMap;
	m_clusterCount = 0;

	m_minimumBounds = vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	m_maximumBounds = vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	// iterate over trajectories
	for (int y = 0; y < m_numberOfTrajectories; y++)
	{
		int cellCount = 0;
		float xValue, yValue;

		std::vector<float> xTemp, yTemp;

		// iterate over time-steps
		for (int x = 0; x < maxTimeSteps; x++)
		{
			// convert string into float values
			xValue = x;
			yValue = m_csvDocument.GetCell<float>(x, y);

			// update bounding volume depending on X and Y values
			m_minimumBounds.x = min(m_minimumBounds.x, xValue);
			m_maximumBounds.x = max(m_maximumBounds.x, xValue);

			m_minimumBounds.y = min(m_minimumBounds.y, yValue);
			m_maximumBounds.y = max(m_maximumBounds.y, yValue);

			xTemp.push_back(xValue);
			yTemp.push_back(yValue);
			cellCount++;

			// GL_LINE_STRIP_ADJACENCY requires the first and last element to be duplicated ------------
			if (x == 0)
			{
				xTemp.push_back(xValue);
				yTemp.push_back(yValue);
				cellCount++;
			}
		}

		// duplicate last item
		xTemp.push_back(xValue);
		yTemp.push_back(yValue);
		cellCount++;
		//----------------------------------------------------------------------------------------------

		// update vector that stores the number of valid time-steps per trajectory
		m_numberOfTimesteps.push_back(cellCount);

		// find out how long the longest trajectory is
		if (cellCount > m_numberOfMaxTimesteps)
			m_numberOfMaxTimesteps = cellCount;

		// update trajectory vectors
		m_XTrajectories.push_back(xTemp);
		m_YTrajectories.push_back(yTemp);

		auto iter = clusterMap.insert(std::pair<std::string, int>(m_csvDocument.GetCell<std::string>(m_csvDocument.GetColumnCount()-1,y), m_clusterCount));

		if (iter.second)
			m_clusterCount++;

		int clusterId = iter.first->second;
		m_clusterIds.push_back(clusterId);
	}
}

void TableData::loadAndrewsSeries(const std::string& filePath)
{

	m_filePath = filePath;

	// extract file name ---------------------------------------------------------
	std::string tempPath = m_filePath;
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
	int maxTimeSteps = m_csvDocument.GetColumnCount() - 1;


	// clear buffers if they already contain data ----------
	m_XTrajectories.clear();
	m_YTrajectories.clear();
	m_numberOfTimesteps.clear();
	m_clusterIds.clear();
	//------------------------------------------------------

	std::unordered_map < std::string, int> clusterMap;
	m_clusterCount = 0;

	m_minimumBounds = vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	m_maximumBounds = vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	// iterate over trajectories
	for (int y = 0; y < m_numberOfTrajectories; y++)
	{
		int cellCount = 0;
		float xValue, yValue;

		std::vector<float> xTemp, yTemp;

		// iterate over time-steps
		for (int x = 0; x < maxTimeSteps; x++)
		{
			// convert string into float values
			xValue = x;
			yValue = m_csvDocument.GetCell<float>(x, y);

			// update bounding volume depending on X and Y values
			m_minimumBounds.x = min(m_minimumBounds.x, xValue);
			m_maximumBounds.x = max(m_maximumBounds.x, xValue);

			m_minimumBounds.y = min(m_minimumBounds.y, yValue);
			m_maximumBounds.y = max(m_maximumBounds.y, yValue);

			xTemp.push_back(xValue);
			yTemp.push_back(yValue);
			cellCount++;
		}
		//----------------------------------------------------------------------------------------------

		// update vector that stores the number of valid time-steps per trajectory
		m_numberOfTimesteps.push_back(cellCount);

		// find out how long the longest trajectory is
		if (cellCount > m_numberOfMaxTimesteps)
			m_numberOfMaxTimesteps = cellCount;

		// update trajectory vectors
		m_XTrajectories.push_back(xTemp);
		m_YTrajectories.push_back(yTemp);

		auto iter = clusterMap.insert(std::pair<std::string, int>(m_csvDocument.GetCell<std::string>(m_csvDocument.GetColumnCount() - 1, y), m_clusterCount));

		if (iter.second)
			m_clusterCount++;

		int clusterId = iter.first->second;
		m_clusterIds.push_back(clusterId);
	}
}

void molumes::TableData::andrewsTransform(int steps, bool normalize)
{

	if (normalize)
	{
		std::vector< vec2 > minmax;
		minmax.resize(m_clusterCount, vec2(FLT_MAX, -FLT_MAX));

		// compute cluster envelopes
		for (int i = 0; i < m_numberOfTrajectories; i++)
		{
			int clusterId = m_clusterIds.at(i);
			const std::vector<float>& values = m_YTrajectories[i];
			vec2& valueMinMax = minmax[clusterId];

			for (int x = 0; x < values.size(); x++)
			{
				valueMinMax.x = std::min(valueMinMax.x, values[x]);
				valueMinMax.y = std::max(valueMinMax.y, values[x]);
			}
		}

		for (int i = 0; i < m_numberOfTrajectories; i++)
		{
			int clusterId = m_clusterIds.at(i);
			std::vector<float>& values = m_YTrajectories[i];
			const vec2& valueMinMax = minmax[clusterId];

			for (int x = 0; x < values.size(); x++)
				values[x] = (values[x] - valueMinMax.x) / (valueMinMax.y - valueMinMax.x);
		}
	}

	m_minimumBounds = vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	m_maximumBounds = vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	m_numberOfMaxTimesteps = steps + 2;

	for (int i = 0; i < m_numberOfTrajectories; i++)
	{
		std::vector<float>& xTrajectory = m_XTrajectories.at(i);
		std::vector<float>& yTrajectory = m_YTrajectories.at(i);

		std::vector<float> xValues;
		std::vector<float> yValues;

		float theta = -pi<float>();
		float increment = 2.0f * pi<float>() / float(steps);

		for (int x=0;x<steps;x++)
		{
			float value = yTrajectory.at(0) / sqrt(2.0f);
			
			for (int j = 1; j < yTrajectory.size(); j++)
			{
				const float scale = ceilf(float(j) * 0.5f);

				if (j % 2 == 0)
					value += yTrajectory.at(j) * cos(scale * theta);
				else
					value += yTrajectory.at(j) * sin(scale * theta);

			}

			xValues.push_back(theta);
			yValues.push_back(value);

			// GL_LINE_STRIP_ADJACENCY requires the first and last element to be duplicated ------------
			if (x == 0)
			{
				xValues.push_back(theta);
				yValues.push_back(value);
			}

			// update bounding volume depending on X and Y values
			m_minimumBounds.x = min(m_minimumBounds.x, theta);
			m_maximumBounds.x = max(m_maximumBounds.x, theta);

			m_minimumBounds.y = min(m_minimumBounds.y, value);
			m_maximumBounds.y = max(m_maximumBounds.y, value);

			theta += increment;
		}

		// duplicate last item
		xValues.push_back(xValues.back());
		yValues.push_back(yValues.back());
		//----------------------------------------------------------------------------------------------

		m_numberOfTimesteps[i] = steps+2;
		xTrajectory.swap(xValues);
		yTrajectory.swap(yValues);
	}
}

vec3 TableData::minimumBounds() const
{
	return m_minimumBounds;
}

vec3 TableData::maximumBounds() const
{
	return m_maximumBounds;
}