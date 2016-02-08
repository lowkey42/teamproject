#version 100
precision mediump float;

attribute vec3 position;
attribute vec2 uv;

varying vec2 uv_frag;

uniform mat4 VP;

void main() {
	gl_Position = VP * vec4(position.xyz, 1);

	uv_frag = uv;
}

