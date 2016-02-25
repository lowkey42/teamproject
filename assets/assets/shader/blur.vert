#version 100
precision lowp float;

attribute vec2 xy;
attribute vec2 uv;

varying vec2 uv_frag_l1;
varying vec2 uv_frag_l2;
varying vec2 uv_frag_r1;
varying vec2 uv_frag_r2;

uniform bool horizontal;
uniform sampler2D texture;
uniform vec2 texture_size;

void main() {
	gl_Position = vec4(xy.x*2.0, xy.y*2.0, 0.0, 1.0);

	vec2 tex_offset = 1.0 / texture_size;
	if(horizontal)
		tex_offset.y=0.0;
	else
		tex_offset.x=0.0;
	
	uv_frag_l1 = uv + 0.53805 * tex_offset;
	uv_frag_l2 = uv + 2.06278 * tex_offset;
	uv_frag_r1 = uv - 0.53805 * tex_offset;
	uv_frag_r2 = uv - 2.06278 * tex_offset;
}
