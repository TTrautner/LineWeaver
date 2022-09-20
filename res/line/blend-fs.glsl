#version 450
#include "/defines.glsl"
#include "/globals.glsl"

layout (location = 0) out vec4 lineChartTexture;

layout(pixel_center_integer) in vec4 gl_FragCoord;

uniform usampler2D offsetTexture;
uniform sampler2D depthTexture;
uniform sampler2D blurTexture;

// weight used for soft depth compositing
uniform float smoothness;

uniform vec3 backgroundColor;

uniform vec2 viewportSize;

uniform vec2 lensPosition;
uniform float lensRadius;
uniform vec3 lensBorderColor;

struct BufferEntry
{
	uint id;
	uint previous;

	float importance;
	vec4 color;
};

layout(std430, binding = 1) buffer intersectionBuffer
{
	uint count;
	BufferEntry intersections[];
};

layout(std430, binding = 2) buffer totalPixelBuffer
{
	// count all pixels per trajectory
	uint totalPixelCounter[];
};

layout(std430, binding = 3) buffer visiblePixelBuffer
{
	// count only top pixels per trajectory 
	uint visiblePixelCounter[];
};

// compute weight dependent on depth value 'z' within 0.1f <= |z| <= 500
// McGuire and Bavoil, 2013: http://jcgt.org/published/0002/02/09/
float computeWeight(float z){
	
	// equation 5:
	//return 1.0f;

	//------------------------------------------------------------

	// equation 7:
	return max(0.01f, min(3000, 10.0f/(0.00001f + pow(abs(z)/5.0f,2) + pow(abs(z)/200.0f,6) ) )); 

	// equation 8:
	//return max(0.01f, min(3000, 10.0f/(0.00001f + pow(abs(z)/10.0f,3) + pow(abs(z)/200.0f,6) ) )); 

	// equation 9:
	//return max(0.01f, min(3000, 0.03f/(0.00001f + pow(abs(z)/200.0f,4) ) )); 
}


void main()
{
	ivec2 fragCoord = ivec2(gl_FragCoord.xy);
	vec4 blurValue = texelFetch(blurTexture,fragCoord,0);

#ifdef RS_LINKEDLIST
	const uint maxEntries = 256;
	uint entryCount = 0;
	uint indices[maxEntries];
	uint indices2[maxEntries];

	uint offset = texelFetch(offsetTexture,ivec2(gl_FragCoord.xy),0).r;

	while (offset > 0)
	{
		indices[entryCount++] = offset;
		offset = intersections[offset].previous;
	}

	vec4 blendedColor = vec4(backgroundColor, 1.0f);
	bool swapped;
	
	// Bubble Sort =================================================================================================	
	if (entryCount > 0)
	{
		for (uint i=0;i<entryCount-1;i++)
		{
			swapped = false;

			for (uint j=0;j<entryCount-i-1;j++)
			{
				if(intersections[indices[j]].importance > intersections[indices[j+1]].importance)
				{
					uint temp = indices[j];
					indices[j] = indices[j+1];
					indices[j+1] = temp;
					swapped = true;
				}
			}

			if (!swapped)
				break;
		}
	}
	//==============================================================================================================

#ifdef CALCULATE_OVERPLOTTING_INDEX
	if(entryCount > 0){

		for(int i = 0; i < entryCount; i++){

			// count all pixel from every trajectory
			atomicAdd(totalPixelCounter[intersections[indices[i]].id], 1);
		}

		// count only pixels from the top most layer
		atomicAdd(visiblePixelCounter[intersections[indices[entryCount-1]].id], 1);
	}
#endif

	// Option 1: naively blend pixels together =====================================================================
	//for(int i = 0; i < entryCount; i++){
	//        blendedColor = porterDuffOverOperator(intersections[indices[i]].color, blendedColor);
	//}

	// Option 2: Hybrid Visibility Composition =====================================================================
	// Bruckner et al., 2010: https://www.cg.tuwien.ac.at/research/publications/2010/bruckner-2010-HVC/

	for (int i=0; i<entryCount; i++){
		
		uint iIndexI = indices[i];
		
        if (intersections[iIndexI].color.a > 0.0f){
			
			vec4 vecSum = vec4(0.0f, 0.0f, 0.0f, 0.0f);
			
			// Optimization (stop when contribution is zero) -- forward direction
            for (int j=i+1; j<entryCount; j++){
				
				uint iIndexJ = indices[j];
 
                if (intersections[iIndexJ].color.a > 0.0f){

					float fDistance = abs(intersections[iIndexI].importance - intersections[iIndexJ].importance);
                    float fFactor = 1.0-smoothstep(0.0,smoothness,fDistance);

                    vec4 vecWeightedColor = intersections[iIndexJ].color * fFactor;
                    vecSum += vecWeightedColor;

					if (fFactor <= 1.0/256.0)
						break;
				}
			}
 
			// Optimization (stop when contribution is zero) -- backward direction
            for (int j=i; j>=0; j--){
				
				uint iIndexJ = indices[j];
 
                if (intersections[iIndexJ].color.a > 0.0f){

					float fDistance = abs(intersections[iIndexI].importance - intersections[iIndexJ].importance);
                    float fFactor = 1.0-smoothstep(0.0,smoothness,fDistance);

                    vec4 vecWeightedColor = intersections[iIndexJ].color * fFactor;
                    vecSum += vecWeightedColor;

					if (fFactor <= 1.0/256.0)
						break;
				}
			}

            if (vecSum.a > 0.0f)
				vecSum /= vecSum.a;
 
            vecSum *= intersections[iIndexI].color.a;
 
            blendedColor = porterDuffOverOperator(vecSum,blendedColor);
		}
    }

	// Option 3: Weighted Blended Order-Independent Transparency ===================================================
	// McGuire and Bavoil, 2013: http://jcgt.org/published/0002/02/09/
	
	//vec3 ci = vec3(0,0,0);
	//float ai = 0.0f;
	//float wi = 0.0f;
	//float alphaProduct = 1.0f;
	//
	//for(int i = 0; i < entryCount; i++){
	//	
	//	float a = intersections[indices[i]].color.a;
	//
	//	// compute weigth using importance which is mapped from [1,0] --> [0.1, 500]
	//	wi = a * computeWeight((1.0f - intersections[indices[i]].importance) * (500.0f-0.1f) + 0.1f);
	//
	//	ci += intersections[indices[i]].color.rgb * wi;
	//	ai += a * wi;
	//
	//	alphaProduct *= (1.0f - a);
	//}
	//
	//blendedColor.rgb = (ci/ai) * (1.0f - alphaProduct) + (backgroundColor*alphaProduct);
	//---------------------------------------------------------------------------------------

	float diff = min(0.0,blurValue.y-blurValue.x);//blurredDepthValue;//depthValue;//abs(depthValue-blurredDepthValue);
	blendedColor.rgb = clamp(blendedColor.rgb+1.5*vec3(diff),vec3(0.0),vec3(1.0));

	lineChartTexture = blendedColor;
#else
	// intentionally, do nothing!
#endif

#if defined(LENS_FEATURE) || defined(ANGULAR_BRUSHING)

	float aspectRatio = viewportSize.x/viewportSize.y;
	vec2 ndCoordinates = (gl_FragCoord.xy-viewportSize/2)/(viewportSize/2);
	float pxlDistance = length((lensPosition-ndCoordinates) * vec2(aspectRatio, 1.0));
	
	// make lens thickness dependent on constant line-width but independent of window size
	float startInner = lensRadius - 15.5f /*empirically chosen*/ / viewportSize.y;
	float endOuter = lensRadius + 15.5f /*empirically chosen*/ / viewportSize.y;
	
	// draw border of lens
	if(pxlDistance >= startInner && pxlDistance <= lensRadius)
	{
		lineChartTexture.rgb = mix(lineChartTexture.rgb, lensBorderColor, smoothstep(startInner, lensRadius, pxlDistance));
	}
	else if (pxlDistance > lensRadius && pxlDistance <= endOuter)
	{
		lineChartTexture.rgb = mix(lineChartTexture.rgb, lensBorderColor, 1.0f - smoothstep(lensRadius, endOuter, pxlDistance));
	}
#endif

}