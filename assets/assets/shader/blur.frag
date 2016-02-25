#version 100
precision lowp float;

varying vec2 uv_frag_l1;
varying vec2 uv_frag_l2;
varying vec2 uv_frag_r1;
varying vec2 uv_frag_r2;

uniform sampler2D texture;


void main() {
	vec3 result = texture2D(texture, uv_frag_l1).rgb * 0.44908
		        + texture2D(texture, uv_frag_l2).rgb * 0.05092
		        + texture2D(texture, uv_frag_r1).rgb * 0.44908
		        + texture2D(texture, uv_frag_r2).rgb * 0.05092;

	gl_FragColor = vec4(result, 1.0);
}
