#version 100
precision mediump float;

attribute vec2 xy;
attribute vec2 uv;

attribute vec3 position;
attribute vec3 direction;
attribute float rotation;
attribute float frames;
attribute float current_frame;
attribute float size;
attribute float alpha;
attribute float opacity;


varying vec2 uv_frag;
varying float alpha_frag;
varying float opacity_frag;


uniform mat4 view;
uniform mat4 proj;


vec2 rotate(vec2 p, float a) {
	vec2 r = mat2(cos(a), -sin(a), sin(a), cos(a)) * p;

	return vec2(r.x, -r.y);
}

void main() {
	vec2 dir_view = (view * vec4(direction, 0.0)).xy;
	float dir_view_len = length(dir_view);
	float rot_dyn = 0.0;
	if(dir_view_len > 0.01) {
		dir_view /= dir_view_len;

		if(abs(dir_view.x)>0.0)
			rot_dyn = atan(dir_view.y, dir_view.x);
		else
			rot_dyn = asin(dir_view.y);
	}

	vec4 pos = view * vec4(position, 1.0);
	pos += vec4(rotate(xy*size,rotation+rot_dyn), 0.0, 0.0);

	gl_Position = proj * pos;

	uv_frag = vec2(uv.x*((current_frame+1.0)/frames), uv.y);
	alpha_frag = alpha;
	opacity_frag = opacity;
}
