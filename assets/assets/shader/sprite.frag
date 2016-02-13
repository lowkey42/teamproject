#version 100
precision mediump float;

struct Dir_light {
	vec3 color;
	float dir;
};
struct Point_light {
	vec3 pos;
	float dir;
	float angle;
	vec3 color;
	float factor;
};

varying vec2 uv_frag;

uniform sampler2D albedo;
uniform sampler2D normal;
uniform sampler2D emission;
uniform sampler2D roughness;
uniform sampler2D metallic;
uniform sampler2D height;

uniform sampler2D shadowmap_1;
uniform sampler2D shadowmap_2;
uniform sampler2D shadowmap_3;
uniform sampler2D shadowmap_4;

uniform sampler2D environment;
uniform sampler2D last_frame;

uniform vec3 light_ambient;
uniform Dir_light light_sun;
uniform Point_light light_0;
uniform Point_light light_1;
uniform Point_light light_2;
uniform Point_light light_3;

void main() {
	vec4 albedo_color = texture2D(albedo, uv_frag);
	if(albedo_color.a < 0.1) {
		discard;
	}

	vec3 color = albedo_color.rgb * light_ambient;

	// TODO

	gl_FragColor = vec4(color, albedo_color.a);
}

