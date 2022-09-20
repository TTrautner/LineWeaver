#version 450
#extension GL_ARB_shading_language_include : require
#include "/defines.glsl"

layout(location = 0) in float xValue;
layout(location = 1) in float yValue;
layout(location = 2) in float importanceValue;

out vsData {
    float pointImportance;
} vsOut;

uniform float xAxisScaling;
uniform float yAxisScaling;

void main()
{

	// allow user to perform axis dependent scaling
	float scaltedX = xValue*xAxisScaling;
	float scaltedY = yValue*yAxisScaling;

	vsOut.pointImportance = importanceValue;

	gl_Position = vec4(scaltedX, scaltedY, 0.0f, 1.0f);
}