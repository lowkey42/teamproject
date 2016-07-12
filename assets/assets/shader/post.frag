#version 100
precision mediump float;

varying vec2 uv_frag;

uniform sampler2D texture;
uniform sampler2D texture_glow;
uniform vec2 texture_size;
uniform float exposure;
uniform float gamma;
uniform float bloom;


vec3 heji_dawson(vec3 color) {
	vec3 X = max(vec3(0.0), color-0.004);
	vec3 mapped = (X*(6.2*X+.5))/(X*(6.2*X+1.7)+0.06);
	return mapped * mapped;
}

vec3 tone_mapping(vec3 color) {
	color *= exposure;
	color = heji_dawson(color);

	// Gamma correction
	color = pow(color, vec3(1.0 / gamma));
	return color;
}

vec3 sample_fxaa() {
	float FXAA_SPAN_MAX = 8.0;
	float FXAA_REDUCE_MUL = 1.0/8.0;
	float FXAA_REDUCE_MIN = 1.0/128.0;

	vec3 rgbNW=texture2D(texture,uv_frag+(vec2(-1.0,-1.0)/texture_size)).xyz;
	vec3 rgbNE=texture2D(texture,uv_frag+(vec2(1.0,-1.0)/texture_size)).xyz;
	vec3 rgbSW=texture2D(texture,uv_frag+(vec2(-1.0,1.0)/texture_size)).xyz;
	vec3 rgbSE=texture2D(texture,uv_frag+(vec2(1.0,1.0)/texture_size)).xyz;
	vec3 rgbM=texture2D(texture,uv_frag).xyz;

	vec3 luma=vec3(0.299, 0.587, 0.114);
	float lumaNW = dot(rgbNW, luma);
	float lumaNE = dot(rgbNE, luma);
	float lumaSW = dot(rgbSW, luma);
	float lumaSE = dot(rgbSE, luma);
	float lumaM  = dot(rgbM,  luma);

	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

	vec2 dir;
	dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
	dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

	float dirReduce = max(
		(lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
		FXAA_REDUCE_MIN);

	float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

	dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
	      max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
	      dir * rcpDirMin)) / texture_size;

	vec3 rgbA = (1.0/2.0) * (
		texture2D(texture, uv_frag.xy + dir * (1.0/3.0 - 0.5)).xyz +
		texture2D(texture, uv_frag.xy + dir * (2.0/3.0 - 0.5)).xyz);
	vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
		texture2D(texture, uv_frag.xy + dir * (0.0/3.0 - 0.5)).xyz +
		texture2D(texture, uv_frag.xy + dir * (3.0/3.0 - 0.5)).xyz);
	float lumaB = dot(rgbB, luma);

	if((lumaB < lumaMin) || (lumaB > lumaMax)){
		return rgbA;
	}else{
		return rgbB;
	}
}

void main() {
	vec3 color = sample_fxaa() + texture2D(texture_glow, uv_frag).rgb*bloom;

	gl_FragColor = vec4(tone_mapping(color), 1.0);
}
