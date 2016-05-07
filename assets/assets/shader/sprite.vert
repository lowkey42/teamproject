#version 100
precision mediump float;

attribute vec3 position;
attribute vec2 uv;
attribute vec4 uv_clip;
attribute vec2 tangent;
attribute float shadow_resistence;

varying vec2 uv_frag;
varying vec4 uv_clip_frag;
varying vec3 pos_frag;
varying vec3 pos_vp_frag;
varying vec3 pos_lvp_frag;
varying float shadow_resistence_frag;

varying mat3 TBN;

uniform mat4 vp;
uniform mat4 vp_light;

void main() {
	vec4 pos_vp = vp * vec4(position, 1);
	vec4 pos_lvp = vp_light * vec4(position, 1);
	gl_Position = pos_vp;

	pos_vp_frag = pos_vp.xyz / pos_vp.w;
	pos_lvp_frag = pos_lvp.xyz / pos_lvp.w;
	uv_frag = uv;
	uv_clip_frag = uv_clip;
	pos_frag = position;
	shadow_resistence_frag = shadow_resistence;

	vec3 T = normalize(vec3(tangent,0.0));
	vec3 N = vec3(0.0,0.0,1.0);
	vec3 B = cross(N, T);
	TBN = mat3(T, B, N);
}

