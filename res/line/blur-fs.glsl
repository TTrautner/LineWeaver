#version 450
#include "/defines.glsl"
#include "/globals.glsl"

uniform sampler2D blurTexture;
uniform float offset;

layout(location = 0) out vec4 blurResult;

void main(void)
{
	const int kernelWidth = 1;
	ivec2 fragCoord = ivec2(gl_FragCoord.xy);

	float value = 0.0;
	float sum = 0.0f;

	float depthValue = texelFetch(blurTexture,fragCoord,0).r;
	vec2 size = textureSize(blurTexture,0);

	for (int j=-kernelWidth;j<=kernelWidth;j++)
	{
		for (int i=-kernelWidth;i<=kernelWidth;i++)
		{
			vec2 coord = (fragCoord.xy+offset*vec2(i,j))/size;
			value += texture(blurTexture,coord,0).g;
			sum += 1.0;
		}
	}

	value /= sum;
	blurResult = vec4(depthValue,value,value,value);

}