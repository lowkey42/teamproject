#version 100
precision mediump float;

attribute vec2 xy;
attribute vec2 uv;

varying float theta;

void main() {
	gl_Position = vec4(xy.x*2.0, xy.y*2.0, 0.0, 1.0);

	const float PI = 3.141;
	theta = uv.x * 2.0 * PI; //< uv.x is our current percentage of the cirlce
}
