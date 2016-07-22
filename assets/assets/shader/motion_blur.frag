#version 100
precision lowp float;


varying vec2 uv_center;
varying vec2 uv_1;
varying vec2 uv_2;
varying vec2 uv_3;

uniform sampler2D texture;


void main() {
	vec3 result = texture2D(texture, uv_center).rgb * (1.0 - 0.2969069646728344 - 0.09447039785044732 - 0.010381362401148057);
	result += texture2D(texture, uv_1).rgb * 0.2969069646728344;
	result += texture2D(texture, uv_2).rgb * 0.09447039785044732;
	result += texture2D(texture, uv_3).rgb * 0.010381362401148057;

	gl_FragColor = vec4(result, 1.0);
}
