#include "Scene.h"

using namespace molumes;

Scene::Scene()
{
	m_tableData = std::make_unique<TableData>();
	m_tableImportance = std::make_unique<TableImportance>();
}

TableData* Scene::tableData()
{
	return m_tableData.get();
}

TableImportance* Scene::tableImportance()
{
	return m_tableImportance.get();
}