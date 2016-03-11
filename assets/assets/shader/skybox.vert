#version 100
precision mediump float;

attribute vec3 position;
varying vec3 position_frag;

uniform mat4 vp;
uniform vec3 eye;

void main() {
	vec3 p = position;
	vec4 pos = vp * vec4(p*15.0 + eye, 1.0);
	gl_Position = pos.xyww;
	position_frag = position;
}
