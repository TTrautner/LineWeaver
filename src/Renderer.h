#pragma once
#include <list>
#include <globjects/base/File.h>

namespace molumes
{
	class Viewer;

	class Renderer
	{
	public:
		Renderer(Viewer* viewer);
		Viewer * viewer();
		void setEnabled(bool enabled);
		bool isEnabled() const;
		virtual std::list<globjects::File*> shaderFiles() const;
		virtual void display() = 0;

	private:
		Viewer* m_viewer;
		bool m_enabled = true;
	};

}