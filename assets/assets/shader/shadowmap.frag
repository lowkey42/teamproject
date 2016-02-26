#version 100
precision mediump float;

varying vec2 uv_frag;

uniform sampler2D occlusions;

uniform vec2 light_positions[4];

uniform float shadowmap_size;

uniform mat4 VP_inv;

vec2 ndc2uv(vec2 p) {
	return p*0.5 + 0.5;
}
vec3 pos2World(vec3 p) {
	return vec3(VP_inv * vec4(p, 1.0));
}

void main() {
	const float PI = 3.141;

	vec2 light_pos;
	if(uv_frag.y < 1.0/4.0) {
		light_pos = light_positions[0];
	} else if(uv_frag.y < 2.0/4.0) {
		light_pos = light_positions[1];
	} else if(uv_frag.y < 3.0/4.0) {
		light_pos = light_positions[2];
	} else if(uv_frag.y < 4.0/4.0) {
		light_pos = light_positions[3];
	}

	float distance = 1000.0;

	float theta = uv_frag.x * 2.0 * PI; //< uv_frag.x is our current percentage of the cirlce

	for (float r=0.0; r<1.0; r+=1.0/shadowmap_size*2.0) {
		vec2 coord = vec2(r * cos(theta), r * sin(theta)) + light_pos;

		//sample the occlusion map
		float data = texture2D(occlusions, ndc2uv(coord)).r;

		if(data>=0.1) {
			vec3 delta = pos2World(vec3(light_pos, 0.0)) - pos2World(vec3(coord, 0.0));
			float d = length(delta.xy);
			distance = min(distance, d);
		}
	}

	gl_FragColor = vec4(distance, distance, distance, 1.0);
}
