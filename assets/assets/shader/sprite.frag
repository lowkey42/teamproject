#version 100
precision mediump float;

varying vec2 uv_frag;

uniform sampler2D texture;

void main() {
	vec4 color = texture2D(texture, uv_frag);
	if(color.a < 0.1){
		discard;
	}
	
	gl_FragColor = color;
}

