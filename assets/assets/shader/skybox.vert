#version 100
precision mediump float;

attribute vec3 position;
varying vec3 position_frag;

uniform mat4 vp;
uniform vec3 eye;

void main() {
	vec3 p = position;
	vec4 pos = vp * vec4(p*100.0 + eye, 1.0);
	gl_Position = pos.xyww;

	float a = -eye.x * 0.0;
	position_frag = vec3(position.x*cos(a)+position.z*sin(a),
	                     position.y,
	                     -position.x*sin(a)+position.z*cos(a));
}
