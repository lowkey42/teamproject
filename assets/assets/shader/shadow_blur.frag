#version 100
precision lowp float;


varying vec2 uv_center;
varying vec2 uv_l1;
varying vec2 uv_l2;
varying vec2 uv_l3;
varying vec2 uv_r1;
varying vec2 uv_r2;
varying vec2 uv_r3;

uniform sampler2D texture;


void main() {
	vec4 center = texture2D(texture, uv_center);

	vec2 result = center.rb * 0.1964825501511404;
	result += texture2D(texture, uv_l1).rb * 0.2969069646728344;
	result += texture2D(texture, uv_l2).rb * 0.09447039785044732;
	result += texture2D(texture, uv_l3).rb * 0.010381362401148057;

	result += texture2D(texture, uv_r1).rb * 0.2969069646728344;
	result += texture2D(texture, uv_r2).rb * 0.09447039785044732;
	result += texture2D(texture, uv_r3).rb * 0.010381362401148057;

	result.r = mix(center.r, result.r, min(1.0,center.g*4.0-0.1));
	result.g = mix(center.b, result.g, min(1.0,center.a*4.0-0.1));

	gl_FragColor = vec4(result.r, center.g, result.g, center.a);
}
