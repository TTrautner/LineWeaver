
const float GOLDEN_RATIO_CONJUGATE = 0.618033988749895f;
const float PI = 3.141592653589793f;

/* source: https://gist.github.com/gre/1650294 and visualizations of the functions: https://easings.net/en
 * also relevant: https://github.com/AndrewRayCode/easing-utils/blob/master/src/easing.js
 * Easing Functions - inspired from http://gizma.com/easing/
 * only considering the t value for the range [0, 1] => [0, 1]
 */
float easingFunction(float t)
{

#ifdef EASE_LINEAR
	// no easing, no acceleration
	return t;
#endif

#ifdef EASE_IN_SINE
	// slight acceleration from zero to full speed
	return (-1)*cos(t*(PI/2))+1;
#endif

#ifdef EASE_OUT_SINE
	// slight deceleration at the end
	return sin(t*(PI/2));
#endif

#ifdef EASE_IN_OUT_SINE
	// slight acceleration at beginning and slight deceleration at end
	return -0.5*(cos(PI*t)-1);
#endif

#ifdef EASE_IN_QUAD
	// accelerating from zero velocity
	return t*t;
#endif

#ifdef EASE_OUT_QUAD
	// decelerating to zero velocity
	return t*(2-t);
#endif

#ifdef EASE_IN_OUT_QUAD
	// acceleration until halfway, then deceleration
	if(t < 0.5f){
		return 2*t*t; 
	}else{
		return -1+(4-2*t)*t;
	}
#endif

#ifdef EASE_IN_CUBIC
	// accelerating from zero velocity 
	return t*t*t;
#endif

#ifdef EASE_OUT_CUBIC
	// decelerating to zero velocity 
	float t1 = t - 1;
    return t1*t1*t1+1;
#endif

#ifdef EASE_IN_OUT_CUBIC
	// acceleration until halfway, then deceleration 
	if(t < 0.5f){
		return 4*t*t*t;
	}else{
		return (t-1)*(2*t-2)*(2*t-2)+1;
	}
#endif

#ifdef EASE_IN_QUART
	// accelerating from zero velocity 
	return t*t*t*t;
#endif

#ifdef EASE_OUT_QUART
	// decelerating to zero velocity 
	float t1 = t-1;
    return 1-t1*t1*t1*t1;
#endif

#ifdef EASE_IN_OUT_QUART
	// acceleration until halfway, then deceleration
	float t1 = t-1;

	if(t < 0.5f){
		return 8*t*t*t*t;
	}else{
		return 1-8*t1*t1*t1*t1;
	}
#endif

#ifdef EASE_IN_QUINT
	// accelerating from zero velocity
	return t*t*t*t*t;
#endif

#ifdef EASE_OUT_QUINT
	// decelerating to zero velocity
	float t1 = t-1;
    return 1+t1*t1*t1*t1*t1;
#endif

#ifdef EASE_IN_OUT_QUINT
	// acceleration until halfway, then deceleration 
	float t1 = t-1;
	if(t < 0.5f){
		return  16*t*t*t*t*t;
	}else{
		return 1+16*t1*t1*t1*t1*t1;
	}
#endif

#ifdef EASE_IN_EXPO
	// accelerate exponentially until finish
	if(t == 0){
        return 0;
    }

    return pow(2, 10*(t-1));
#endif

#ifdef EASE_OUT_EXPO
	// initial exponential acceleration slowing to stop
	if(t == 1){
        return 1;
    }

    return (-pow(2, -10*t)+1);
#endif

#ifdef EASE_IN_OUT_EXPO
	// exponential acceleration and deceleration
	if(t == 0 || t == 1) {
        return t;
    }

    float scaledTime = t*2;
    float scaledTime1 = scaledTime-1;

    if( scaledTime < 1 ) {
        return 0.5*pow(2, 10*scaledTime1);
    }

    return 0.5*(-pow( 2, -10*scaledTime1)+2);
#endif

	// if no parameter is set, return t
	return t;
}

// All components are in the range [0…1], including hue.
// implementation taken from: https://stackoverflow.com/questions/15095909/from-rgb-to-hsv-in-opengl-glsl
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

// All components are in the range [0…1], including hue.
// implementation taken from: https://stackoverflow.com/questions/15095909/from-rgb-to-hsv-in-opengl-glsl
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

// Generate random colors using the golden ratio for equal spacing in color-space
// implementation taken from: https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
vec3 getRandomizedColors(float randomValue){

	float h = randomValue * GOLDEN_RATIO_CONJUGATE;
	h = mod(h,1.0f);

	return hsv2rgb(vec3(h, 0.75f, 1.0f));
}

// See Porter-Duff 'A over B' operators: https://de.wikipedia.org/wiki/Alpha_Blending
vec4 porterDuffOverOperator(vec4 vecF, vec4 vecB){
	return vecF + (1.0-vecF.a)*vecB;
}