#include "RenderingStrategy.h"
using namespace molumes;

RenderingStrategy::RenderingStrategy(TableData* data)
{
	m_dataTable = data;
}

std::vector<float>& RenderingStrategy::activeXColumn() {
	return m_activeXColumn;
}

std::vector<float>& RenderingStrategy::activeYColumn() {
	return m_activeYColumn;
}

std::vector<float>& RenderingStrategy::activeImportance() {
	return m_activeImportance;
}

// fill the zipped vector with pairs consisting of the corresponding elements of a and b. (This assumes that the vectors have equal length)
// source: https://stackoverflow.com/questions/37368787/c-sort-one-vector-based-on-another-one
void RenderingStrategy::zip(const std::vector<float> &a, const std::vector<int> &b, std::vector<std::pair<float, int>> &zipped) {
	for (size_t i = 0; i < a.size(); ++i) {
		zipped.push_back(std::make_pair(a[i], b[i]));
	}
}

// write the first and second element of the pairs in the given zipped vector into a and b. (This assumes that the vectors have equal length)
// source: https://stackoverflow.com/questions/37368787/c-sort-one-vector-based-on-another-one
void RenderingStrategy::unzip(const std::vector<std::pair<float, int>> &zipped, std::vector<float> &a, std::vector<int> &b) {
	for (size_t i = 0; i < a.size(); i++) {
		a[i] = zipped[i].first;
		b[i] = zipped[i].second;
	}
}

// fill the zipped vector with pairs consisting of the corresponding elements of a and b. (This assumes that the vectors have equal length)
// source: https://stackoverflow.com/questions/37368787/c-sort-one-vector-based-on-another-one
void RenderingStrategy::zip(const std::vector<int> &a, const std::vector<int> &b, std::vector<std::pair<int, int>> &zipped) {
	for (size_t i = 0; i < a.size(); ++i) {
		zipped.push_back(std::make_pair(a[i], b[i]));
	}
}

// write the first and second element of the pairs in the given zipped vector into a and b. (This assumes that the vectors have equal length)
// source: https://stackoverflow.com/questions/37368787/c-sort-one-vector-based-on-another-one
void RenderingStrategy::unzip(const std::vector<std::pair<int, int>> &zipped, std::vector<int> &a, std::vector<int> &b) {
	for (size_t i = 0; i < a.size(); i++) {
		a[i] = zipped[i].first;
		b[i] = zipped[i].second;
	}
}