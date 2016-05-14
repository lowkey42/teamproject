#version 100
precision mediump float;

struct Dir_light {
	vec3 color;
	vec3 dir;
};
struct Point_light {
	vec3 pos;
	float dir;
	float angle;
	vec3 color;
	vec3 factors;
};

varying vec2 uv_frag;
varying vec4 uv_clip_frag;
varying vec3 pos_frag;

uniform sampler2D albedo_tex;
uniform sampler2D material_tex;

uniform samplerCube environment_tex;

uniform float light_ambient;
uniform Dir_light light_sun;
uniform Point_light light[2];

uniform vec3 eye;

uniform vec4 background_tint;


float G1V ( float dotNV, float k ) {
	return 1.0 / (dotNV*(1.0 - k) + k);
}
vec3 calc_light(vec3 light_dir, vec3 light_color, vec3 normal, vec3 albedo, vec3 view_dir, float roughness, float metalness, float reflectance) {

	// TODO: cleanup this mess and replace experiments with real formulas
	vec3 N = normal;
	vec3 V = -view_dir;

	float alpha = roughness*roughness;
	vec3 L = normalize(light_dir);
	vec3 H = normalize (V + L);

	float dotNL = clamp (dot (N, L), 0.0, 1.0);
	float dotNV = clamp (dot (N, V), 0.0, 1.0);
	float dotNH = clamp (dot (N, H), 0.0, 1.0);
	float dotLH = clamp (dot (L, H), 0.0, 1.0);

	float D, vis;
	vec3 F;

	// NDF : GGX
	float alphaSqr = alpha*alpha;
	float pi = 3.1415926535;
	float denom = dotNH * dotNH *(alphaSqr - 1.0) + 1.0;
	D = alphaSqr / (pi * denom * denom);

	// Fresnel (Schlick)
	vec3 F0 = mix(vec3(0.16*reflectance*reflectance), albedo.rgb, 0.0);
	float dotLH5 = pow (1.0 - dotLH, 5.0);
	F = F0 + (1.0 - F0)*(dotLH5);

	// Visibility term (G) : Smith with Schlick's approximation
	float k = alpha / 2.0;
	vis = G1V (dotNL, k) * G1V (dotNV, k);

	vec3 specular = D * F * vis;

	float invPi = 0.31830988618;
	vec3 diffuse = (albedo * invPi);

	diffuse*=(1.0 - metalness);

	light_color = mix(light_color, light_color*albedo, metalness);

	specular*=(1.0 + metalness*20.0); //< not physicaly accurate, but makes metals more shiny

	return (diffuse + specular) * light_color * dotNL;
}

float my_smoothstep(float edge0, float edge1, float x) {
	x = clamp((x-edge0)/(edge1-edge0), 0.0, 1.0);
	return x*x*x*(x*(x*6.0 -15.0) + 10.0);
}

vec3 calc_point_light(Point_light light, vec3 normal, vec3 albedo, vec3 view_dir, float roughness, float metalness, float reflectance) {
	vec3 light_dir = light.pos.xyz - pos_frag;
	float light_dist = length(light_dir);
	light_dir /= light_dist;

	float attenuation = clamp(1.0 / (light.factors.x + light_dist*light.factors.y + light_dist*light_dist*light.factors.z)-0.01, 0.0, 1.0);

	const float PI = 3.141;
	float theta = atan(light_dir.y, -light_dir.x)-light.dir;
	theta = ((theta/(2.0*PI)) - floor(theta/(2.0*PI))) * 2.0*PI - PI;

	float max_angle = (light.angle + my_smoothstep(1.8*PI, 2.0*PI, light.angle)*0.2) / 2.0;
	attenuation *= my_smoothstep(0.0, 0.1, clamp(max_angle-abs(theta), -1.0, 1.0));

	return calc_light(light_dir, light.color, normal, albedo, view_dir, roughness, metalness, reflectance) * attenuation;
}
vec3 calc_dir_light(Dir_light light, vec3 normal, vec3 albedo, vec3 view_dir, float roughness, float metalness, float reflectance) {
	return calc_light(light.dir, light.color, normal, albedo, view_dir, roughness, metalness, reflectance);
}

void main() {
	vec2 uv = mod(uv_frag, 1.0) * (uv_clip_frag.zw-uv_clip_frag.xy) + uv_clip_frag.xy;

	vec4 albedo = pow(texture2D(albedo_tex, uv), vec4(2.2));
	vec3 normal = vec3(0,0,1);

	vec3 material = texture2D(material_tex, uv).xyz;
	float emmision = material.r;
	float metalness = material.g;
	float smoothness = 1.0-material.b;

	float roughness = 1.0 - smoothness*smoothness;
	float reflectance = clamp((0.9-roughness)*1.1 + metalness*0.2, 0.0, 1.0);

	if(albedo.a < 0.01) {
		discard;
	}

	vec3 view_dir = normalize(pos_frag-eye);

	vec3 ambient = pow(textureCube(environment_tex, normal, 10.0).rgb, vec3(2.2));
	vec3 color = albedo.rgb * ambient* min(light_ambient, 1.0);

	color += calc_dir_light(light_sun, normal, albedo.rgb, view_dir, roughness, metalness, reflectance);


	for(int i=0; i<2; i++) {
		color += calc_point_light(light[i], normal, albedo.rgb, view_dir, roughness, metalness, reflectance);
	}

	vec3 refl = ambient;
	refl *= mix(vec3(1,1,1), albedo.rgb, metalness);
	color += refl*reflectance*0.1;

	color += albedo.rgb*3.0*emmision;

	color = mix(color, background_tint.rgb, background_tint.a * min((-pos_frag.z-10.0)/10.0, 1.0));

	gl_FragColor = vec4(color, albedo.a);
}

