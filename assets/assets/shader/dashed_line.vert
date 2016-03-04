#version 100
precision mediump float;

attribute float index;

uniform mat4 vp;
uniform vec2 p1;
uniform vec2 p2;

varying vec2 position_frag;

void main() {
	vec2 pos = index<=0.0 ? p1 : p2;

	gl_Position = vp * vec4(pos, 0.0, 1.0);

	position_frag = pos;
}
