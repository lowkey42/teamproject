#version 100
precision mediump float;

varying vec2 uv_frag;
varying vec2 uv_mask_frag;

uniform sampler2D texture;
uniform vec3 color;


vec4 blend_mask(vec4 x, vec4 y) {
	return vec4(vec3(1.0) - (vec3(1.0)-x.rgb) * (vec3(1.0)-y.rgb), 1.0) * x.a;
}

void main() {
	if(uv_frag.x<=0.0 || uv_frag.y<=0.0 || uv_frag.x>=0.5 || uv_frag.y>=1.0) {
		discard;
		return;
	}

	vec4 base_c = texture2D(texture, uv_frag);
	base_c.rgb = base_c.rgb * color;
	vec4 c = blend_mask(base_c,
	                    texture2D(texture, uv_mask_frag)*vec4(0.33));

	gl_FragColor = vec4(c.rgb, 0.0);
}
