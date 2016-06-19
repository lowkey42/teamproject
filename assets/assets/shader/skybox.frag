#version 100
precision mediump float;

varying vec3 position_frag;

uniform vec3 tint;
uniform float brightness;
uniform samplerCube texture;

void main() {
	gl_FragColor = vec4(pow(textureCube(texture, position_frag).rgb, vec3(2.2))*tint*brightness, 1.0);
}

