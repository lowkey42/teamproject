#version 100
precision mediump float;

varying vec2 uv_frag;
varying vec4 color_frag;

uniform sampler2D texture;

void main() {
	vec4 c = texture2D(texture, uv_frag) * color_frag;

	if(c.a>0.01)
		gl_FragColor = c;
	else
		discard;
}
