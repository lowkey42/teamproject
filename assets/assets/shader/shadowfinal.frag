#version 100
precision mediump float;

varying vec2 uv_frag;

uniform sampler2D shadowmaps_tex;
uniform vec2 light_positions[2];


float sample_shadow_ray(vec2 tc, float r, vec4 channel_mask) {
	vec4 shadow_data = texture2D(shadowmaps_tex, tc);
	float d = dot(shadow_data, channel_mask)*4.0;
	return step(r, d);
}

float sample_shadow(float r, vec3 dir, vec4 channel_mask) {
	const float PI = 3.141;
	float theta = atan(dir.y, dir.x) + PI;
	vec2 tc = vec2(theta /(2.0*PI),0.5);

	return clamp(sample_shadow_ray(tc, r, channel_mask), 0.0, 1.0);
}

vec2 light(vec2 pos, vec4 channel_mask) {
	vec3 light_dir_linear = vec3(pos - uv_frag.xy, 0);
	float light_dist_linear = length(light_dir_linear);
	light_dir_linear /= light_dist_linear;

	return vec2(sample_shadow(light_dist_linear, light_dir_linear, channel_mask), abs(light_dist_linear)/4.0);
}

void main() {
	gl_FragColor.rg = light(light_positions[0], vec4(1,0,0,0));
	gl_FragColor.ba = light(light_positions[1], vec4(0,1,0,0));
}
