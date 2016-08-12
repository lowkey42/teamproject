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
varying vec2 decals_uv_frag;
varying vec3 pos_frag;
varying vec2 shadowmap_uv_frag;
varying vec2 hue_change_frag;
varying float shadow_resistence_frag;
varying float decals_intensity_frag;
varying mat3 TBN;

uniform sampler2D albedo_tex;
uniform sampler2D normal_tex;
uniform sampler2D material_tex;
uniform sampler2D height_tex;

uniform sampler2D shadowmaps_tex;

uniform sampler2D decals_tex;
uniform samplerCube environment_tex;
uniform sampler2D last_frame_tex;

uniform float light_ambient;
uniform Dir_light light_sun;
uniform Point_light light[8];

uniform vec3 eye;
uniform float alpha_cutoff;

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

	float spec_mod_factor = 2.0 + metalness*40.0; //< not physicaly accurate, but makes metals more shiny
	D*=mix(spec_mod_factor, 0.0, roughness); // TODO(this is a workaround): specular is to powerfull for realy rough surfaces
	D = clamp(D, 0.0, 50.0);

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

	return (diffuse + specular) * light_color * dotNL;
}

float my_smoothstep(float edge0, float edge1, float x) {
	x = clamp((x-edge0)/(edge1-edge0), 0.0, 1.0);
	return x*x*x*(x*(x*6.0 -15.0) + 10.0);
}

float calc_shadow(int light_num) {
	vec4 shadow = texture2D(shadowmaps_tex, shadowmap_uv_frag);
	if(light_num>0) {
		shadow.r = shadow.b;
	}

	return mix(shadow.r, 1.0, shadow_resistence_frag*0.9);
}

vec3 calc_point_light(Point_light light, vec3 normal, vec3 albedo, vec3 view_dir, float roughness, float metalness, float reflectance) {
	vec3 light_dir = light.pos.xyz - pos_frag;
	float light_dist = length(light_dir);
	light_dir /= light_dist;

	float attenuation = clamp(1.0 / (light.factors.x + light_dist*light.factors.y + light_dist*light_dist*light.factors.z)-0.01, 0.0, 1.0);

	const float PI = 3.141;
	float theta = atan(light_dir.y, -light_dir.x)-light.dir;
	theta = ((theta/(2.0*PI)) - floor(theta/(2.0*PI))) * 2.0*PI - PI;

	float max_angle = (light.angle + my_smoothstep(1.8*PI, 2.0*PI, light.angle)*1.0) / 2.0;
	attenuation *= my_smoothstep(-0.2, 0.6, clamp(max_angle-abs(theta), -1.0, 1.0));

	return calc_light(light_dir, light.color, normal, albedo, view_dir, roughness, metalness, reflectance) * attenuation;
}
vec3 calc_dir_light(Dir_light light, vec3 normal, vec3 albedo, vec3 view_dir, float roughness, float metalness, float reflectance) {
	return calc_light(light.dir, light.color, normal, albedo, view_dir, roughness, metalness, reflectance);
}

vec3 rgb2hsv(vec3 c) {
	vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	vec4 p = c.g < c.b ? vec4(c.bg, K.wz) : vec4(c.gb, K.xy);
	vec4 q = c.r < p.x ? vec4(p.xyw, c.r) : vec4(c.r, p.yzx);

	float d = q.x - min(q.w, q.y);
	float e = 1.0e-10;
	return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}
vec3 hsv2rgb(vec3 c) {
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float luminance(vec3 c) {
	vec3 f = vec3(0.299,0.587,0.114);
	return sqrt(c.r*c.r*f.r + c.g*c.g*f.g + c.b*c.b*f.b);
}

vec3 hue_shift(vec3 in_color) {
	vec3 hsv = rgb2hsv(in_color);
	hsv.x = abs(hsv.x-hue_change_frag.x)<0.01 ? hue_change_frag.y : hsv.x;
	return hsv2rgb(hsv);
}
vec4 read_albedo(vec2 uv) {
	vec4 c = texture2D(albedo_tex, uv);
	c.rgb = hue_shift(pow(c.rgb, vec3(2.2))) * c.a;
	return c;
}

void main() {
	vec2 uv = mod(uv_frag, 1.0) * (uv_clip_frag.zw-uv_clip_frag.xy) + uv_clip_frag.xy;

	vec4 albedo = read_albedo(uv);
	vec3 normal = texture2D(normal_tex, uv).xyz;
	if(length(normal)<0.00001)
		normal = vec3(0,0,1);
	else {
		normal = normalize(normal*2.0 - 1.0);
	}
	normal = TBN * normal;

	vec3 material = texture2D(material_tex, uv).xyz;
	float emmision = material.r;
	float metalness = material.g;
	float smoothness = 1.0-material.b;

	if(albedo.a < alpha_cutoff) {
		discard;
	}

	float decals_fade = clamp(1.0+pos_frag.z/2.0, 0.25, 1.0) * decals_intensity_frag * albedo.a;
	vec4 decals = texture2D(decals_tex, decals_uv_frag);
	albedo.rgb = mix(albedo.rgb, decals.rgb * decals_fade, decals.a * decals_fade);
	emmision = mix(emmision, 0.3, max(0.0, decals.a * decals_fade - 0.5));
	smoothness = mix(smoothness, 0.4, decals.a * decals_fade);
	metalness = mix(metalness, 0.6, decals.a * decals_fade);


	float roughness = 1.0 - smoothness*smoothness;
	float reflectance = clamp((0.9-roughness)*1.1 + metalness*0.1, 0.0, 1.0);

	vec3 view_dir = normalize(pos_frag-vec3(eye.xy, eye.z*10.0));

	vec3 ambient = (pow(textureCube(environment_tex, normal, 10.0).rgb, vec3(2.2)) * light_ambient);
	vec3 color = vec3(0.0);

	for(int i=0; i<2; i++) {
		color += calc_point_light(light[i], normal, albedo.rgb, view_dir, roughness, metalness, reflectance) * calc_shadow(i);
	}
	for(int i=2; i<8; i++) {
		color += calc_point_light(light[i], normal, albedo.rgb, view_dir, roughness, metalness, reflectance);
	}

	// in low-light scene, discard colors but keep down-scaled luminance
	color = mix(color, vec3(my_smoothstep(0.1, 0.2, pow(luminance(albedo.rgb), 3.3)*6000.0))*0.007, my_smoothstep(0.015, 0.005, length(color)));

	color += albedo.rgb * ambient;
	color += calc_dir_light(light_sun, normal, albedo.rgb, view_dir, roughness, metalness, reflectance);

	vec3 refl = pow(textureCube(environment_tex, reflect(view_dir,normal), 10.0*roughness).rgb, vec3(2.2));
	refl *= mix(vec3(1,1,1), albedo.rgb, metalness);
	color += refl*reflectance*0.2;

	color = mix(color, albedo.rgb*3.0, emmision);

	gl_FragColor = vec4(color, albedo.a);
}

