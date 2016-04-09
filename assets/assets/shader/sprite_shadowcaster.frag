#version 100
precision mediump float;

varying vec2 uv_frag;
varying vec4 uv_clip_frag;
varying vec3 pos_frag;

uniform sampler2D albedo_tex;
uniform sampler2D height_tex;


void main() {
	vec2 uv = mod(uv_frag, 1.0) * (uv_clip_frag.zw-uv_clip_frag.xy) + uv_clip_frag.xy;

	float alpha  = texture2D(albedo_tex, uv).a;
	float height = texture2D(height_tex, uv).r;

	//height= 1.0;// TODO: remove and blur instead to remove jitter

	if(alpha < 0.1 || height < 0.7) {
		discard;
	}

	gl_FragColor = vec4(1.0);
}

