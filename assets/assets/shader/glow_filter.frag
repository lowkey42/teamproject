#version 100
precision mediump float;

varying vec2 uv_frag;

uniform sampler2D texture;

void main() {
	vec3 color = texture2D(texture, uv_frag).rgb;
	float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722)) * 1.0;

	if(brightness>0.01)
		gl_FragColor = vec4(color * clamp(brightness,0.0,1.0), 1.0);
	else
		gl_FragColor = vec4(0.0,0.0,0.0, 1.0);
}
