#version 100
precision mediump float;

varying vec2 uv_frag;
varying vec3 pos_frag;

uniform sampler2D albedo_tex;
uniform sampler2D height_tex;


void main() {
	float alpha  = texture2D(albedo_tex, uv_frag).a;
	float height = texture2D(height_tex, uv_frag).r;

	//height= 1.0;// TODO: remove and blur instead to remove jitter

	if(alpha < 0.1 || height < 0.7) {
		discard;
	}

	gl_FragColor = vec4(1.0);
}

