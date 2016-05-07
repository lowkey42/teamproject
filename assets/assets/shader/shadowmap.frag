#version 100
precision mediump float;

varying vec2 uv_frag;

uniform sampler2D occlusions;

uniform vec2 light_positions[8];


uniform mat4 VP_inv;

vec2 ndc2uv(vec2 p) {
	return clamp(p*0.5 + 0.5, vec2(0.0, 0.0), vec2(1.0, 1.0));
}
vec3 pos2World(vec3 p) {
	return vec3(VP_inv * vec4(p, 1.0));
}

void main() {
	const float PI = 3.141;

	vec2 light_pos;
	if(uv_frag.y < 1.0/8.0) {
		light_pos = light_positions[0];
	} else if(uv_frag.y < 2.0/8.0) {
		light_pos = light_positions[1];
	} else if(uv_frag.y < 3.0/8.0) {
		light_pos = light_positions[2];
	} else if(uv_frag.y < 4.0/8.0) {
		light_pos = light_positions[3];
	} else if(uv_frag.y < 5.0/8.0) {
		light_pos = light_positions[4];
	} else if(uv_frag.y < 6.0/8.0) {
		light_pos = light_positions[5];
	} else if(uv_frag.y < 7.0/8.0) {
		light_pos = light_positions[6];
	} else if(uv_frag.y < 8.0/8.0) {
		light_pos = light_positions[7];
	} else {
		light_pos = vec2(99999, 99999);
	}

	float distance = 4.0;

	float theta = uv_frag.x * 2.0 * PI; //< uv_frag.x is our current percentage of the cirlce

	for (float r=0.0; r<4.0; r+=1.0/(1024.0)) {
		vec2 coord = vec2(r * cos(theta), r * sin(theta)) + light_pos;

		//sample the occlusion map
		float data = texture2D(occlusions, ndc2uv(coord)).r;

		if(data>=0.1) {
			//vec3 delta = pos2World(vec3(light_pos, 0.0)) - pos2World(vec3(coord, 0.0));
			//float d = length(delta.xy);
			distance = min(distance, r);
			break;
		}
	}

	distance = clamp(distance/4.0, 0.0, 1.0);

	gl_FragColor = vec4(distance, distance, distance, 1.0);
}
