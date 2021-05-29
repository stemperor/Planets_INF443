
#version 330 core

in struct fragment_data
{
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 uv;

	vec3 eye;
} fragment;

layout(location=0) out vec4 FragColor;

uniform sampler2D image_texture;

uniform vec3 light = vec3(1.0, 1.0, 1.0);

uniform vec3 color = vec3(1.0, 1.0, 1.0); // Unifor color of the object
uniform float Ka = 0.4; // Ambient coefficient
uniform bool use_texture = true;
uniform bool texture_inverse_y = false;


void main()
{

	vec2 uv_image = vec2(fragment.uv.x, 1.0-fragment.uv.y);
	if(texture_inverse_y) {
		uv_image.y = 1.0-uv_image.y;
	}
	vec4 color_image_texture = texture(image_texture, uv_image);
	if(use_texture==false) {
		color_image_texture=vec4(1.0,1.0,1.0,1.0);
	}
	vec3 color_object  = fragment.color * color * color_image_texture.rgb;
	vec3 color_shading = Ka * color_object;
	
	FragColor = vec4(color_shading, color_image_texture.a);
}
