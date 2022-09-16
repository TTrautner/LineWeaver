#include "LinkedListRendering.h"

using namespace lineweaver;
using namespace gl;
using namespace glm;

LinkedListRendering::LinkedListRendering(TableData* data) : RenderingStrategy(data)
{

}


void LinkedListRendering::prepareDataBuffers() {

	// fill buffers, our technique does not require a specific order or sorting
	for (int i = 0; i < m_dataTable->m_numberOfTrajectories; i++) {

		for (int j = 0; j < m_dataTable->m_numberOfTimesteps.at(i); j++) {
			m_activeXColumn.push_back(m_dataTable->m_XTrajectories.at(i).at(j));
			m_activeYColumn.push_back(m_dataTable->m_YTrajectories.at(i).at(j));
		}
	}
}

// fill the importance buffer using a data dependent metric
void LinkedListRendering::prepareImportanceBuffer() {

	//==================================================== GLOBAL LENGTH ============================================================

	std::vector<float> lengthOfTrajectories;

	// compute the length of all currently loaded trajectories
	for (int i = 0; i < m_dataTable->m_numberOfTrajectories; i++) {

		float vectorLenght = 0.0f;

		for (int j = 1; j < m_dataTable->m_numberOfTimesteps.at(i); j++) {

			float x = m_dataTable->m_XTrajectories.at(i).at(j) - m_dataTable->m_XTrajectories.at(i).at(j - 1);
			float y = m_dataTable->m_YTrajectories.at(i).at(j) - m_dataTable->m_YTrajectories.at(i).at(j - 1);
			vectorLenght += sqrt(pow(x, 2) + pow(y, 2));
		}

		lengthOfTrajectories.push_back(vectorLenght);
		vectorLenght = 0.0f;
	}

	// create an index vector
	std::vector<int> index;
	for (int i = 0; i < m_dataTable->m_numberOfTrajectories; i++) { index.push_back(i); }

	// zip the vectors together
	std::vector<std::pair<float, int>> zipped;
	zip(lengthOfTrajectories, index, zipped);

	// sort the vector of pairs
	std::sort(std::begin(zipped), std::end(zipped),
		[&](const auto& a, const auto& b)
		{
			return a.first > b.first;
		});

	// write the sorted pairs back to the original vectors
	unzip(zipped, lengthOfTrajectories, index);

	std::vector<float> tempImportance;

	for (int i = 0; i < m_dataTable->m_numberOfTrajectories; i++) {

		std::vector<int>::iterator it;
		it = find(index.begin(), index.end(), i);

		// store the position of the iterator
		int position = std::distance(index.begin(), it);

		for (int j = 0; j < m_dataTable->m_numberOfTimesteps.at(i); j++) {

			// normalize importance
			tempImportance.push_back(float(position) / float(m_dataTable->m_numberOfTrajectories - 1));
		}
	}

	m_activeImportance = tempImportance;
}

void LinkedListRendering::prepareImportanceBuffer(TableImportance* importance) {

	// store importance data and fill buffers
	m_importanceTable = importance;
	
	//make sure to clean up before pushing new values to it
	m_activeImportance.clear();

	for (int i = 0; i < m_importanceTable->m_numberOfTrajectories; i++) {

		for (int j = 0; j < m_importanceTable->m_numberOfTimesteps.at(i); j++) {
			m_activeImportance.push_back(m_importanceTable->m_importance.at(i).at(j));
		}
	}
}

// check if a point is on the LEFT side of an edge
bool inside(vec2 p, vec2 p1, vec2 p2)
{
	return (p2.y - p1.y) * p.x + (p1.x - p2.x) * p.y + (p2.x * p1.y - p1.x * p2.y) < 0.0f;
}

// calculate intersection point
vec2 intersection(vec2 cp1, vec2 cp2, vec2 s, vec2 e)
{
	vec2 dc = { cp1.x - cp2.x, cp1.y - cp2.y };
	vec2 dp = { s.x - e.x, s.y - e.y };

	float n1 = cp1.x * cp2.y - cp1.y * cp2.x;
	float n2 = s.x * e.y - s.y * e.x;
	float n3 = 1.0 / (dc.x * dp.y - dc.y * dp.x);

	return { (n1 * dp.x - n2 * dc.x) * n3, (n1 * dp.y - n2 * dc.y) * n3 };
}

// Sutherland-Hodgman clipping
void clip(std::vector<vec2> & resultPolygon, const std::vector<vec2>& subjectPolygon, const std::vector<vec2> & clipPolygon)
{
	vec2 cp1, cp2, s, e;
	std::vector<vec2> inputPolygon;
	resultPolygon = subjectPolygon;


	for (int j = 0; j < clipPolygon.size(); j++)
	{
		// copy new polygon to input polygon & set counter to 0
		inputPolygon = resultPolygon;
		resultPolygon.clear();

		// get clipping polygon edge
		cp1 = clipPolygon[j];
		cp2 = clipPolygon[(j + 1) % clipPolygon.size()];

		for (int i = 0; i < inputPolygon.size(); i++)
		{
			// get subject polygon edge
			s = inputPolygon[i];
			e = inputPolygon[(i + 1) % inputPolygon.size()];

			// Case 1: Both vertices are inside:
			// Only the second vertex is added to the output list
			if (inside(s, cp1, cp2) && inside(e, cp1, cp2))
				resultPolygon.push_back(e);

			// Case 2: First vertex is outside while second one is inside:
			// Both the point of intersection of the edge with the clip boundary
			// and the second vertex are added to the output list
			else if (!inside(s, cp1, cp2) && inside(e, cp1, cp2))
			{
				resultPolygon.push_back(intersection(cp1, cp2, s, e));
				resultPolygon.push_back(e);
			}

			// Case 3: First vertex is inside while second one is outside:
			// Only the point of intersection of the edge with the clip boundary
			// is added to the output list
			else if (inside(s, cp1, cp2) && !inside(e, cp1, cp2))
				resultPolygon.push_back(intersection(cp1, cp2, s, e));

			// Case 4: Both vertices are outside
			else if (!inside(s, cp1, cp2) && !inside(e, cp1, cp2))
			{
				// No vertices are added to the output list
			}
		}
	}
}

float area(const std::vector<vec2> & polygon)
{
	float area = 0.0f;

	int j = polygon.size() - 1;

	for (int i = 0; i < polygon.size(); i++)
	{
		area += (polygon[j].x + polygon[i].x) * (polygon[j].y - polygon[i].y);
		j = i;
	}

	return abs(area / 2.0);
}

float overlap(vec2 aCurr, vec2 aNext, vec2 bCurr, vec2 bNext)
{
	std::vector<vec2> polyA;
	polyA.push_back(vec2(0.0f, aCurr.x));
	polyA.push_back(vec2(1.0f, aNext.x));
	polyA.push_back(vec2(1.0f, aNext.y));
	polyA.push_back(vec2(0.0f, aCurr.y));

	std::vector<vec2> polyB;
	polyB.push_back(vec2(0.0f, bCurr.x));
	polyB.push_back(vec2(1.0f, bNext.x));
	polyB.push_back(vec2(1.0f, bNext.y));
	polyB.push_back(vec2(0.0f, bCurr.y));

	std::vector<vec2> resultPoly;
	clip(resultPoly,polyA, polyB);

	float overlap = area(resultPoly);
	return overlap;
}

void LinkedListRendering::weaveSeries(const TableData& table)
{
/*
	std::vector<vec2> sub,clp,res;
	sub.push_back(vec2(50,150));
	sub.push_back(vec2(200, 50));
	sub.push_back(vec2(350, 150));

	sub.push_back(vec2(350, 300));
	sub.push_back(vec2(250, 300));
	sub.push_back(vec2(200, 250));

	sub.push_back(vec2(150, 350));
	sub.push_back(vec2(100, 250));
	sub.push_back(vec2(100, 200));

	clp.push_back(vec2(100, 100));
	clp.push_back(vec2(300, 100));
	clp.push_back(vec2(300, 300));
	clp.push_back(vec2(100, 300));

	clip(res, sub, clp);

	for (int i = 0; i < res.size(); i++)
		std::cout << "(" << res[i].x << ", " << res[i].y << ")" << std::endl;

	std::cout << "Area:" << area(res) << std::endl;
*/
	int valueCount = INT_MAX;
	int clusterCount = table.m_clusterCount;
	int trajectoryCount = table.m_numberOfTrajectories;

	std::vector< std::vector<vec2> > envelopes;
	envelopes.resize(clusterCount);

	// compute cluster envelopes
	for (int i = 0; i < table.m_clusterIds.size(); i++)
	{
		int clusterId = table.m_clusterIds.at(i);
		std::vector<vec2>& envelope = envelopes[clusterId];

		const std::vector<float>& values = table.m_YTrajectories[i];
		envelope.resize(values.size(), vec2(FLT_MAX, -FLT_MAX));

		for (int x = 0; x < values.size(); x++)
		{
			vec2& valueMinMax = envelope[x];
			valueMinMax.x = std::min(valueMinMax.x, values[x]);
			valueMinMax.y = std::max(valueMinMax.y, values[x]);
		}

		valueCount = std::min(valueCount, int(values.size()));
	}

	std::vector<float> clusterSizes;
	clusterSizes.resize(clusterCount, 0.0f);

	std::vector<float> clusterOverlaps;
	clusterOverlaps.resize(int(clusterCount * clusterCount), 0.0f);

	std::vector<bool> clusterActive;
	clusterActive.resize(clusterCount, false);

	std::vector<float> clusterCosts;
	clusterCosts.resize(clusterCount, 0.0f);

	std::vector<int> resultSet;
	resultSet.resize(clusterCount, -1);

	std::vector<int> resultOrder;
	resultOrder.resize(clusterCount, -1);

	std::vector< std::vector<int> > clusterOrders;

	// iterate over all x values
	for (int x = 0; x < valueCount; x++)
	{
		// compute cluster sizes for current x value
		for (int clusterId = 0; clusterId < clusterCount; clusterId++)
		{
			const std::vector<vec2>& envelope = envelopes[clusterId];
			const vec2& valueMinMax = envelope[x];
			const float clusterSize = valueMinMax.y - valueMinMax.x;
			clusterSizes[clusterId] = clusterSize + 1.0; // avoid issues with zero size
		}

		// compute overlap matrix for current x value
		for (int clusterIdJ = 0; clusterIdJ < clusterCount; clusterIdJ++)
		{
			const std::vector<vec2>& envelopeJ = envelopes[clusterIdJ];
			const vec2 valueMinMaxJ = envelopeJ[x];
			const vec2 valueMinMaxJNext = x < valueCount-1 ? envelopeJ[x+1] : valueMinMaxJ;

			for (int clusterIdI = 0; clusterIdI < clusterIdJ; clusterIdI++)
			{
				const std::vector<vec2>& envelopeI = envelopes[clusterIdI];
				const vec2 valueMinMaxI = envelopeI[x];
				const vec2 valueMinMaxINext = x < valueCount-1 ? envelopeI[x+1] : valueMinMaxI;

				float minValue = std::max(valueMinMaxI.x, valueMinMaxJ.x);
				float maxValue = std::min(valueMinMaxI.y, valueMinMaxJ.y);

				float clusterOverlap = 0.0f;

				if (minValue <= maxValue)
					clusterOverlap = maxValue - minValue;

				float oldOverlap = clusterOverlap;
				/*
				clusterOverlap = overlap(valueMinMaxJ, valueMinMaxJNext, valueMinMaxI, valueMinMaxINext);

				if (oldOverlap != clusterOverlap)
				{
					std::cout << oldOverlap << " != " << clusterOverlap << std::endl;

				}*/

				clusterOverlaps[clusterIdI + clusterIdJ * clusterCount] = clusterOverlap;
				clusterOverlaps[clusterIdJ + clusterIdI * clusterCount] = clusterOverlap;
			}
		}

		std::fill(clusterActive.begin(), clusterActive.end(), true);
		std::fill(resultSet.begin(), resultSet.end(), -1);
		std::fill(resultOrder.begin(), resultOrder.end(), -1);


		// "cost" function for each cluster = sum of overlaps with all other active clusters / cluster size
		for (int i = 0; i < clusterCount; i++)
		{
			for (int clusterIdJ = 0; clusterIdJ < clusterCount; clusterIdJ++)
			{
				if (clusterActive[clusterIdJ])
				{
					float clusterCost = 0.0f;

					for (int clusterIdI = 0; clusterIdI < clusterCount; clusterIdI++)
					{
						if (clusterIdI != clusterIdJ)
						{
							if (clusterActive[clusterIdI])
								clusterCost += clusterOverlaps[clusterIdI + clusterIdJ * clusterCount];
						}
					}

					clusterCost *= clusterSizes[clusterIdJ];
					clusterCosts[clusterIdJ] = clusterCost;
				}
			}


			// find active cluster that minimizes cost function
			int minimumIndex = 0;
			float minimumCost = FLT_MAX;

			for (int clusterIdI = 0; clusterIdI < clusterCount; clusterIdI++)
			{
				if (clusterActive[clusterIdI])
				{
					if (clusterCosts[clusterIdI] < minimumCost)
					{
						minimumCost = clusterCosts[clusterIdI];
						minimumIndex = clusterIdI;
					}
				}
			}

			// add cluster to result set and mark as inactive
			clusterActive[minimumIndex] = false;

			resultSet[i] = minimumIndex;
			resultOrder[minimumIndex] = i;
		}

		clusterOrders.push_back(resultOrder);
	}

	m_activeImportance.clear();

	std::vector< std::vector<float> > importance;

	// set importance values
	for (int i = 0; i < trajectoryCount; i++)
	{
		const std::vector<float>& trajectory = table.m_YTrajectories.at(i);
		int clusterId = table.m_clusterIds.at(i);
		std::vector<float> trajectoryImportance;

		for (int x = 0; x < valueCount; x++)
		{
			int timestepOrder = clusterOrders.at(x).at(clusterId);
			float timestepImportance = 1.0f - float(timestepOrder+1) / float(clusterCount+2);
			trajectoryImportance.push_back(timestepImportance);
		}

		importance.push_back(trajectoryImportance);
	}

	// smooth importance values
	for (int i = 0; i < trajectoryCount; i++)
	{
		std::vector<float>& trajectoryImportance = importance.at(i);
		std::vector<float> smoothedImportance;

		for (int x = 0; x < valueCount; x++)
		{
			const int windowWidth = valueCount / 32;

			float value = 0.0f;
			float sum = 0.0f;

			for (int k = x - windowWidth; k <= x + windowWidth; k++)
			{
				if (k >= 0 && k < valueCount - 1)
				{
					value += trajectoryImportance.at(k);
					sum += 1.0;
				}
			}

			value /= sum;
			smoothedImportance.push_back(value);
		}

		trajectoryImportance.swap(smoothedImportance);
	}

	// copy importance values
	for (int i = 0; i < trajectoryCount; i++)
	{
		const std::vector<float>& trajectoryImportance = importance.at(i);

		for (int x = 0; x < valueCount; x++)
			m_activeImportance.push_back(trajectoryImportance.at(x));
	}

}

void LinkedListRendering::performRendering(globjects::Program* p, globjects::VertexArray* va) {

	int firstIndex = 0;

	// render line segments exactly as they are stored in buffer
	for (int i = 0; i < m_dataTable->m_numberOfTrajectories; i++) {

		p->setUniform("numberOfTimesteps", m_dataTable->m_numberOfTimesteps[i]);
		p->setUniform("trajectoryID", i);

		va->drawArrays(GL_LINE_STRIP_ADJACENCY, firstIndex, m_dataTable->m_numberOfTimesteps[i]);
		firstIndex += m_dataTable->m_numberOfTimesteps[i];
	}
}