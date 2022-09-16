# Line Weaver

Repository: https://github.com/TTrautner/LineWeaver

### Abstract
Line charts are an effective and widely used technique for visualizing series of ordered two-dimensional data points. 
The relationship between consecutive points is thereby indicated by connecting line segments, revealing potential trends 
or clusters in the underlying data. However, when dealing with an increasing number of lines, the render order has 
a major influence on the resulting visualization. Rendering transparent lines can help but unfortunately the blending order 
is currently either ignored or naively used, for example, assuming it is implicitly given by the order in which 
the data was saved in a file. Because of the non-commutativity of classical alpha blending, this produces contradicting 
visualizations of the same underlying data set, so-called "hallucinators". In this paper, we therefore present line weaver, 
a novel visualization technique for dense line charts. Using an importance function, we developed a technique that correctly 
considers the blending order independent of the render order and without any prior sorting of the data. We allow for importance 
functions which are either explicitly given or implicitly derived from the geometric properties of the data, if no external data 
is available. The importance can then be applied globally to entire lines, or locally per pixel which simultaneously supports 
various types of user interaction. Finally, we discuss the potential of our contribution based on different synthetic and 
real-world data sets where classic or naive approaches would fail.


## Setup on Windows

### Prerequisites

The project uses [CMake](https://cmake.org/) and relies on the following libraries: 

- [GLFW](https://www.glfw.org/) 3.3 or higher (https://github.com/glfw/glfw.git) for windowing and input support
- [glm](https://glm.g-truc.net/) 0.9.9.5 or higher (https://github.com/g-truc/glm.git) for its math funtionality
- [glbinding](https://github.com/cginternals/glbinding) 3.1.0 or higher (https://github.com/cginternals/glbinding.git) for OpenGL API binding
- [globjects](https://github.com/cginternals/globjects) 2.0.0 or higher (https://github.com/cginternals/globjects.git) for additional OpenGL wrapping
- [Dear ImGui](https://github.com/ocornut/imgui) 1.71 or higher (https://github.com/ocornut/imgui.git) for GUI elements
- [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/) 3.3.9 or higher (https://git.code.sf.net/p/tinyfiledialogs/code) for dialog functionality

- Microsoft Visual Studio 2015 or 2017 (2017 is recommended as it offers CMake integration)

### Setup

- Open a shell and run ./fetch-libs.cmd to download all dependencies.
- Run ./build-libs.cmd to build the dependencies.
- Run ./configure.cmd to create the Visual Studio solution files (only necessary for Visual Studion versions prior to 2017).
- Open ./build/lineweaver.sln in Visual Studio (only necessary for Visual Studion versions prior to 2017, in VS 2017 the CMakeFiles.txt can be opened directly).

### License

Copyright (c) 2021, Thomas Trautner. Released under the [GPLv3 License](LICENSE.md).
Please visite https://vis.uib.no/team/thomas-trautner/ for contact information.
