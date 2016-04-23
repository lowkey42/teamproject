#version 100
precision mediump float;

varying vec2 uv_frag;

uniform sampler2D texture;
uniform sampler2D texture_glow;
uniform float exposure;
uniform float gamma;
uniform float bloom;


vec3 tone_mapping(vec3 color) {
	// OptimizedHejiDawson
	color *= exposure;
	vec3 X = max(vec3(0.0), color-0.004);
	vec3 mapped = (X*(6.2*X+.5))/(X*(6.2*X+1.7)+0.06);
	mapped*=mapped;

	// Gamma correction
	mapped = pow(mapped, vec3(1.0 / gamma));
	return mapped;
}

void main() {
	vec4 color = texture2D(texture, uv_frag) + texture2D(texture_glow, uv_frag)*bloom;
	
	gl_FragColor = vec4(tone_mapping(color.rgb), 1.0);
}
