#version 100
precision mediump float;

varying vec2 uv_frag;
varying float alpha_frag;
varying float opacity_frag;

uniform sampler2D texture;

void main() {
	vec4 c = texture2D(texture, uv_frag);

	gl_FragColor = vec4(c.rgb, opacity_frag) * c.a * alpha_frag;
}
