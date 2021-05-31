#version 330 core

in struct fragment_data
{
    vec3 position;
	vec3 position_true;
    vec3 normal;
    vec3 color;
    vec2 uv;

	vec3 eye;
} fragment;

layout(location=0) out vec4 FragColor;

uniform sampler2D image_texture;

uniform float sat_radius;
uniform vec3 sat_pos;

void main()
{

// No special light treatment here: saturn's rings probably interact weirdly with light anyways.

	float dist = length(fragment.position);
	
	vec4 color_image_texture = vec4(1,1,1,0);
	
	float fro_ring = 0.7;
	
	if(dist >= fro_ring && dist <= 1.0){
		vec2 uv = vec2( (dist - fro_ring) / (1.0-fro_ring), 0.0);
		color_image_texture = texture(image_texture, uv);
	
	}

	// very basic shadow check
	
	float dist_true = length(fragment.position_true);
	float dist_sat = length(sat_pos);
	
	vec3 sun_dir = normalize(fragment.position_true);
	vec3 d_to_sat = sat_pos - fragment.position_true;
	float ray_dist = length(d_to_sat - dot(d_to_sat, sun_dir)*sun_dir);
	
	if(ray_dist < sat_radius && dist_sat < dist_true){
		color_image_texture = vec4(0,0,0, color_image_texture.a);
	}
	
	FragColor = color_image_texture;
}
