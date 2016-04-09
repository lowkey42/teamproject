#version 100
precision mediump float;

attribute vec3 position;
attribute vec2 uv;
attribute vec4 uv_clip;
attribute float shadow_resistence;

varying vec2 uv_frag;
varying vec4 uv_clip_frag;
varying vec3 pos_frag;
varying float shadow_resistence_frag;

uniform mat4 vp;

void main() {
	gl_Position = vp * vec4(position, 1);

	uv_frag = uv;
	uv_clip_frag = uv_clip;
	pos_frag = position;
	shadow_resistence_frag = shadow_resistence;
}

