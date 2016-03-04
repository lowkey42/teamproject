#version 100
precision mediump float;

attribute vec3 position;
attribute vec2 uv;
attribute float shadow_resistence;

varying vec2 uv_frag;
varying vec3 pos_frag;
varying float shadow_resistence_frag;

uniform mat4 vp;

void main() {
	gl_Position = vp * vec4(position, 1);

	uv_frag = uv;
	pos_frag = position;
	shadow_resistence_frag = shadow_resistence;
}

