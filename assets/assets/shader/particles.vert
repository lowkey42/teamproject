#version 100
precision mediump float;

attribute vec3 position;
attribute vec3 direction;
attribute float rotation;
attribute float frames;
attribute float current_frame;
attribute float size;
attribute float alpha;
attribute float opacity;
attribute float hue_change_out;

varying float frames_frag;
varying float current_frame_frag;
varying float rotation_frag;
varying float alpha_frag;
varying float opacity_frag;
varying float hue_change_out_frag;

uniform mat4 view;
uniform mat4 vp;

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

	vec4 clip_space_pos = vp * vec4(position, 1.0);
	clip_space_pos/=clip_space_pos.w;

	gl_Position = clip_space_pos;
	gl_PointSize = (1.0-clip_space_pos.z) * size * 200.0;

	frames_frag = frames;
	current_frame_frag = floor(current_frame);
	rotation_frag = rotation+rot_dyn;
	alpha_frag = alpha;
	opacity_frag = opacity;
	hue_change_out_frag = hue_change_out;
}
