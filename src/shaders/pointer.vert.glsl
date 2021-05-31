#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;

out struct fragment_data
{
    vec3 position;
    vec2 uv;
} fragment;

uniform mat4 model;
uniform mat4 view;

void main()
{
	fragment.position = vec3(position);
	fragment.uv = uv;

	gl_Position = view * model * vec4(position, 1.0);
}
