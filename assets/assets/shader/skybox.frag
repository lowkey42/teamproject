#version 100
precision mediump float;

varying vec3 position_frag;

uniform samplerCube texture;

void main() {
	gl_FragColor = vec4(pow(textureCube(texture, position_frag).rgb, vec3(2.2)), 1.0);
}

