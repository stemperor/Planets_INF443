
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


void main()
{

	vec2 uv_image = vec2(fragment.uv.x, 1.0-fragment.uv.y);
	
	vec4 color_image_texture = texture(image_texture, uv_image);
	
	FragColor = color_image_texture;
}
