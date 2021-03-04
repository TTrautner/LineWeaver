#pragma once
#include "Table.h"

namespace molumes
{
	class TableData : public Table {

	public:
		TableData();

		// regular series
		void load(const std::string& filePath);
		void loadSeries(const std::string& filePath);

		// andrews plot
		void loadAndrewsSeries(const std::string& filePath);
		void andrewsTransform(int steps = 128, bool normalize = false);

		// access current bounding volume
		glm::vec3 minimumBounds() const;
		glm::vec3 maximumBounds() const;

		std::vector<std::vector<float>> m_XTrajectories;					// [trajectories,timesteps]
		std::vector<std::vector<float>> m_YTrajectories;					// [trajectories,timesteps]
		std::vector<int> m_clusterIds;
		int m_clusterCount = 0;

	private:
		// bounding box of current selection
		glm::vec3 m_minimumBounds = glm::vec3(0.0f);
		glm::vec3 m_maximumBounds = glm::vec3(0.0f);
	};
}