#version 100
precision mediump float;

attribute vec3 position;
varying vec3 position_frag;

uniform mat4 vp;
uniform vec3 eye;

void main() {
	float a = eye.x * 0.001;
	vec3 p = vec3(position.x*cos(a)+position.z*sin(a),
	                     position.y,
	                     -position.x*sin(a)+position.z*cos(a));

	vec4 pos = vp * vec4(p*10000.0 + eye, 1.0);
	gl_Position = pos.xyww;

	position_frag = position;
}
