
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
uniform sampler2D image_night_texture;
uniform sampler2D spec_texture;
uniform sampler2D bump_texture;

uniform vec3 light = vec3(1.0, 1.0, 1.0);

uniform vec3 color = vec3(1.0, 1.0, 1.0); // Unifor color of the object
uniform float alpha = 1.0f; // alpha coefficient
uniform float Ka = 0.4; // Ambient coefficient
uniform float Kd = 0.8; // Diffuse coefficient
uniform float Ks = 0.4f;// Specular coefficient
uniform float specular_exp = 64.0; // Specular exponent
uniform bool use_texture = true;
uniform bool texture_inverse_y = false;


void main()
{

	vec2 uv_image = vec2(fragment.uv.x, 1.0-fragment.uv.y);
	
	
	vec3 N = normalize(fragment.normal);
	vec3 N0 = N;
	
	vec3 tanX = vec3(N.x, -N.z, N.y);
	vec3 tanY = vec3(N.z, N.y, -N.x);
	vec3 tanZ = vec3(-N.y, N.x, N.z);
	vec3 blended_tangent = (tanX +  
                         tanY +  
                         tanZ )/3; 
						 
						 
	vec3 bump = texture(bump_texture, uv_image).rgb;

  vec3 normalTex = bump * 2.0 - 1.0;
  normalTex.xy *= 5.0;
  normalTex.y *= -1.;
  normalTex = normalize( normalTex );
  mat3 tsb = mat3( normalize( blended_tangent ), 
                   normalize( cross( N, blended_tangent ) ), 
                   normalize( N ) );
  
  N = normalize(tsb * normalTex);
	
	
	
	
	
	if (gl_FrontFacing == false) {
		N = -N;
	}
	vec3 L = normalize(light-fragment.position);

	float diffuse = max(dot(N,L),0.0);
	float diffuse0 = max(dot(N0, L), 0.0);
	float specular = 0.0;
	if(diffuse>0.0){
		vec3 R = reflect(-L,N);
		vec3 V = normalize(fragment.eye-fragment.position);
		specular = pow( max(dot(R,V),0.0), specular_exp );
	}


	
	if(texture_inverse_y) {
		uv_image.y = 1.0-uv_image.y;
	}
	vec4 color_image_texture = texture(image_texture, uv_image);
	if(use_texture==false) {
		color_image_texture=vec4(1.0,1.0,1.0,1.0);
	}
	
	
	float Ks2 = Ks * texture(spec_texture, uv_image).x;
	
	vec3 color_object  = fragment.color * color * color_image_texture.rgb;
	vec3 night_color = fragment.color * color * texture(image_night_texture, uv_image).rgb;
	
	float nigth_day_transition = 6;
	
	vec3 color_shading = (Ka + Kd * diffuse) * color_object + Kd*max(0, (1-diffuse0*nigth_day_transition))*night_color + Ks2 * specular * vec3(1.0, 1.0, 1.0);
	
	FragColor = vec4(color_shading, alpha * color_image_texture.a);
}
