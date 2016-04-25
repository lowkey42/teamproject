#version 100
precision mediump float;

struct Dir_light {
	vec3 color;
	vec3 dir;
};
struct Point_light {
	vec3 pos;
	vec2 pos_ndc;
	float dir;
	float angle;
	vec3 color;
	vec3 factors;
};

varying vec2 uv_frag;
varying vec4 uv_clip_frag;
varying vec3 pos_frag;
varying vec3 pos_vp_frag;
varying float shadow_resistence_frag;
varying mat3 TBN;

uniform sampler2D albedo_tex;
uniform sampler2D normal_tex;
uniform sampler2D material_tex;
uniform sampler2D height_tex;

uniform sampler2D shadowmaps_tex;

uniform samplerCube environment_tex;
uniform sampler2D last_frame_tex;

uniform float light_ambient;
uniform Dir_light light_sun;
uniform Point_light light[4];

uniform vec3 eye;

float G1V ( float dotNV, float k ) {
	return 1.0 / (dotNV*(1.0 - k) + k);
}
vec3 calc_light(vec3 light_dir, vec3 light_color, vec3 pos, vec3 normal, vec3 albedo, float roughness, float metalness, float reflectance) {

	// TODO: cleanup this mess and replace experiments with real formulas

	vec3 view_dir = normalize(pos-eye);
	vec3 N = normal;
	vec3 V = normalize(pos-eye*vec3(1,1,-1));

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


	diffuse*=1.0-metalness;
	vec3 refl = pow(textureCube(environment_tex, reflect(view_dir,N), 6.0*roughness).rgb, vec3(2.2)) * 0.25;
	diffuse = mix(diffuse, refl, reflectance);

	light_color = mix(light_color, light_color*albedo, metalness);

	return (diffuse + specular) * light_color * dotNL;
}

float my_smoothstep(float edge0, float edge1, float x) {
	x = clamp((x-edge0)/(edge1-edge0), 0.0, 1.0);
	return x*x*(3.0-2.0*x);
}

float sample_shadow_ray(vec2 tc, float r) {
	float d = texture2D(shadowmaps_tex, tc).r*4.0;
	if(r>d) return 1.0;
	return my_smoothstep(0.1, 0.2, r-d); // TODO: causes light bleeding
}

float sample_shadow(float light_num, float r, vec3 dir) {
	const float PI = 3.141;
	float theta = atan(dir.y, dir.x) + PI;
	vec2 tc = vec2(theta /(2.0*PI),(light_num+0.5)/4.0);

	//the center tex coord, which gives us hard shadows
	float center = sample_shadow_ray(tc,r);

	//we multiply the blur amount by our distance from center
	//this leads to more blurriness as the shadow "fades away"
	float blur = 1.0/512.0 * my_smoothstep(0.0, 1.0, r);

	//now we use a simple gaussian blur
	float sum = 0.0;

	sum += sample_shadow_ray(vec2(tc.x - 4.0*blur, tc.y), r) * 0.05;
	sum += sample_shadow_ray(vec2(tc.x - 3.0*blur, tc.y), r) * 0.09;
	sum += sample_shadow_ray(vec2(tc.x - 2.0*blur, tc.y), r) * 0.12;
	sum += sample_shadow_ray(vec2(tc.x - 1.0*blur, tc.y), r) * 0.15;

	sum += center * 0.18;

	sum += sample_shadow_ray(vec2(tc.x + 1.0*blur, tc.y), r) * 0.15;
	sum += sample_shadow_ray(vec2(tc.x + 2.0*blur, tc.y), r) * 0.12;
	sum += sample_shadow_ray(vec2(tc.x + 3.0*blur, tc.y), r) * 0.09;
	sum += sample_shadow_ray(vec2(tc.x + 4.0*blur, tc.y), r) * 0.05;

	return 1.0-clamp(sum, 0.0, 1.0);
}

vec3 calc_point_light(Point_light light, vec3 pos, vec3 normal, vec3 albedo, float roughness, float metalness, float reflectance, float light_num) {
	vec3 light_dir = light.pos.xyz - pos;
	float light_dist = length(light_dir);
	light_dir /= light_dist;

	vec3 light_dir_linear = vec3(light.pos_ndc.xy - pos_vp_frag.xy, 0);
	float light_dist_linear = length(light_dir_linear);
	light_dir_linear /= light_dist_linear;


	float attenuation = clamp(1.0 / (light.factors.x + light_dist*light.factors.y + light_dist*light_dist*light.factors.z), 0.0, 1.0);

	attenuation *= mix(sample_shadow(light_num, light_dist_linear, light_dir_linear), 1.0, shadow_resistence_frag*0.9);

	const float PI = 3.141;
	float theta = atan(light_dir.y, -light_dir.x)-light.dir;
	theta = ((theta/(2.0*PI)) - floor(theta/(2.0*PI))) * 2.0*PI - PI;

	float max_angle = (light.angle + my_smoothstep(1.8*PI, 2.0*PI, light.angle)*0.2) / 2.0;
	attenuation *= my_smoothstep(0.0, 0.1, clamp(max_angle-abs(theta), -1.0, 1.0));

	return calc_light(light_dir, light.color, pos, normal, albedo, roughness, metalness, reflectance) * attenuation;
}
vec3 calc_dir_light(Dir_light light, vec3 pos, vec3 normal, vec3 albedo, float roughness, float metalness, float reflectance) {
	return calc_light(light.dir, light.color, pos, normal, albedo, roughness, metalness, reflectance);
}

vec3 saturation(vec3 c, float change) {
	vec3 f = vec3(0.299,0.587,0.114);
	float p = sqrt(c.r*c.r*f.r + c.g*c.g*f.g + c.b*c.b*f.b);

	return vec3(p) + (c-vec3(p))*vec3(change);
}

void main() {
	vec2 uv = mod(uv_frag, 1.0) * (uv_clip_frag.zw-uv_clip_frag.xy) + uv_clip_frag.xy;

	vec4 albedo = pow(texture2D(albedo_tex, uv), vec4(2.2));
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
	float smoothness = clamp(1.0-material.b, 0.01, 0.99);

	float roughness = 1.0 - smoothness*smoothness;
	float reflectance = clamp((smoothness-0.1)*1.1 + metalness*0.2, 0.0, 1.0);

	if(albedo.a < 0.01) {
		discard;
	}

	vec3 ambient = (pow(textureCube(environment_tex, normal, 10.0).rgb, vec3(2.2)) * light_ambient);
	vec3 color = albedo.rgb * ambient;

	color += calc_dir_light(light_sun, pos_frag, normal, albedo.rgb, roughness, metalness, reflectance);


	for(int i=0; i<4; i++) {
		color += calc_point_light(light[i], pos_frag, normal, albedo.rgb, roughness, metalness, reflectance, float(i));
	}

	// remove saturation of objects further away
	float fade_out = clamp(-pos_frag.z*0.025, 0.0, 0.5);
	color = saturation(mix(color, ambient*10.0, fade_out*0.2), 1.0 - fade_out);

	gl_FragColor = vec4(color + albedo.rgb*3.0*emmision, albedo.a);
}

