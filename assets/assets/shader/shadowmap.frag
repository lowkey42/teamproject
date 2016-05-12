#version 100
precision mediump float;

varying float theta;
uniform sampler2D occlusions;

uniform vec2 light_positions[8];


vec2 ndc2uv(vec2 p) {
	return clamp(p*0.5 + 0.5, vec2(0.0, 0.0), vec2(1.0, 1.0));
}

void main() {
	vec2 distance = vec2(4.0, 4.0);

	vec2 dir = vec2(cos(theta), sin(theta));

	for (float r=0.0; r<2.0; r+=1.0/(1024.0)) {
		vec2 coord1 = r*dir + light_positions[0];
		vec2 coord2 = r*dir + light_positions[1];

		//sample the occlusion map
		if(texture2D(occlusions, ndc2uv(coord1)).r>=0.1) {
			distance.x = min(distance.x, r);
		}

		if(texture2D(occlusions, ndc2uv(coord2)).r>=0.1) {
			distance.y = min(distance.y, r);
		}
	}

	distance = clamp(distance/vec2(4.0), vec2(0.0), vec2(1.0));

	gl_FragColor = vec4(distance, 0.0, 1.0);
}
