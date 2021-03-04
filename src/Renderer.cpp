#include "Renderer.h"
using namespace molumes;

Renderer::Renderer(Viewer* viewer) : m_viewer(viewer)
{

}

Viewer * Renderer::viewer()
{
	return m_viewer;
}

void Renderer::setEnabled(bool enabled)
{
	m_enabled = enabled;
}

bool Renderer::isEnabled() const
{
	return m_enabled;
}

std::list<globjects::File*> Renderer::shaderFiles() const
{
	return std::list<globjects::File*>();
}