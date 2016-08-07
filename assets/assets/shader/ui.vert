#version 100
precision mediump float;

attribute vec2 position;
attribute vec2 uv;
attribute vec4 color;

varying vec2 uv_frag;
varying vec4 color_frag;

uniform mat4 vp;

void main() {
	gl_Position = vp * vec4(position, 0, 1);
	uv_frag = uv;
	color_frag = color;
}
