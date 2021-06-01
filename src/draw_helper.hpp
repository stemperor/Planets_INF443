#ifndef DRAW_HELPER_H
#define DRAW_HELPER_H


#include "vcl/vcl.hpp"

#include "Dual_Camera.h"

using namespace vcl;

GLuint queryid1;
GLuint queryid2;

struct scene_environment
{
	Dual_camera camera;
	mat4 projection;
	vec3 light;
	bool translate_drawing = true;
};

// The following file contains many functions for drawing. We didn't find how to customize the existing draw function so we made our own for each shader/object
// Some may not be necessary anymore / could be merged, but they were created over time and work as they are now.



template <typename SCENE>
void draw(mesh_drawable const& drawable, SCENE const& scene, bool expected);

// Simply takes into consideration the new makeshift "translate_drawing" parameter of scene
void opengl_uniform(GLuint shader, scene_environment const& current_scene)
{
	opengl_uniform(shader, "projection", current_scene.projection);
	if (current_scene.translate_drawing)
		opengl_uniform(shader, "view", current_scene.camera.matrix_view());
	else
		opengl_uniform(shader, "view", current_scene.camera.matrix_view_or_only());
	opengl_uniform(shader, "light", current_scene.light, false);
}


// Same as the draw functon but with the expected parameter
template <typename SCENE>
void draw(mesh_drawable const& drawable, SCENE const& scene, bool expected)
{
	// Setup shader
	assert_vcl(drawable.shader != 0, "Try to draw mesh_drawable without shader");
	assert_vcl(drawable.texture != 0, "Try to draw mesh_drawable without texture");
	glUseProgram(drawable.shader); opengl_check;

	// Send uniforms for this shader
	opengl_uniform(drawable.shader, scene);
	opengl_uniform(drawable.shader, drawable.shading, expected);
	opengl_uniform(drawable.shader, "model", drawable.transform.matrix());

	// Set texture
	glActiveTexture(GL_TEXTURE0); opengl_check;
	glBindTexture(GL_TEXTURE_2D, drawable.texture); opengl_check;
	opengl_uniform(drawable.shader, "image_texture", 0);  opengl_check;

	// Call draw function
	assert_vcl(drawable.number_triangles > 0, "Try to draw mesh_drawable with 0 triangles"); opengl_check;
	glBindVertexArray(drawable.vao);   opengl_check;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawable.vbo.at("index")); opengl_check;
	glDrawElements(GL_TRIANGLES, GLsizei(drawable.number_triangles * 3), GL_UNSIGNED_INT, nullptr); opengl_check;

	// Clean buffers
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

// Earth has multiple textures and its own shader.
template <typename SCENE>
void drawearth(mesh_drawable const& drawable, SCENE const& scene, GLuint night_texture, GLuint spec_texture, GLuint bump_texture)
{
	// Setup shader
	assert_vcl(drawable.shader != 0, "Try to draw mesh_drawable without shader");
	assert_vcl(drawable.texture != 0, "Try to draw mesh_drawable without texture");

	glUseProgram(drawable.shader); opengl_check;

	// Send uniforms for this shader
	opengl_uniform(drawable.shader, scene);
	opengl_uniform(drawable.shader, drawable.shading, false);
	opengl_uniform(drawable.shader, "model", drawable.transform.matrix());

	// Set texture
	glActiveTexture(GL_TEXTURE0); opengl_check;
	glBindTexture(GL_TEXTURE_2D, drawable.texture); opengl_check;
	opengl_uniform(drawable.shader, "image_texture", 0);  opengl_check;

	glActiveTexture(GL_TEXTURE0 + 1); opengl_check; // Texture unit 1
	glBindTexture(GL_TEXTURE_2D, spec_texture); opengl_check;
	opengl_uniform(drawable.shader, "spec_texture", 1); opengl_check;

	glActiveTexture(GL_TEXTURE0 + 2); opengl_check; // Texture unit 1
	glBindTexture(GL_TEXTURE_2D, bump_texture); opengl_check;
	opengl_uniform(drawable.shader, "bump_texture", 2); opengl_check;

	glActiveTexture(GL_TEXTURE0 + 3); opengl_check; // Texture unit 1
	glBindTexture(GL_TEXTURE_2D, night_texture); opengl_check;
	opengl_uniform(drawable.shader, "image_night_texture", 3); opengl_check;

	// Call draw function
	assert_vcl(drawable.number_triangles > 0, "Try to draw mesh_drawable with 0 triangles"); opengl_check;
	glBindVertexArray(drawable.vao);   opengl_check;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawable.vbo.at("index")); opengl_check;
	glDrawElements(GL_TRIANGLES, GLsizei(drawable.number_triangles * 3), GL_UNSIGNED_INT, nullptr); opengl_check;

	// Clean buffers
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

// The sun needs time and takes no shading parameters
template <typename SCENE>
void drawsun(mesh_drawable const& drawable, SCENE const& scene, double t)
{
	// Setup shader
	assert_vcl(drawable.shader != 0, "Try to draw mesh_drawable without shader");
	glUseProgram(drawable.shader); opengl_check;

	// Send uniforms for this shader
	opengl_uniform(drawable.shader, scene);
	opengl_uniform(drawable.shader, "model", drawable.transform.matrix());
	opengl_uniform(drawable.shader, "t", (float)t, false);



	// Call draw function
	assert_vcl(drawable.number_triangles > 0, "Try to draw mesh_drawable with 0 triangles"); opengl_check;
	glBindVertexArray(drawable.vao);   opengl_check;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawable.vbo.at("index")); opengl_check;

	glDrawElements(GL_TRIANGLES, GLsizei(drawable.number_triangles * 3), GL_UNSIGNED_INT, nullptr); opengl_check;

	// Clean buffers
	glBindVertexArray(0);
}

// To draw the lens flare
template <typename SCENE>
void drawsunshine(mesh_drawable const& drawable, SCENE const& scene, double turn, float sun_occlusion)
{
	// Setup shader
	assert_vcl(drawable.shader != 0, "Try to draw mesh_drawable without shader");
	glUseProgram(drawable.shader); opengl_check;

	// Send uniforms for this shader
	opengl_uniform(drawable.shader, scene);
	opengl_uniform(drawable.shader, "model", drawable.transform.matrix());
	opengl_uniform(drawable.shader, "turn", (float)turn, false);
	opengl_uniform(drawable.shader, "sun_occlusion", (float)sun_occlusion, false);


	glActiveTexture(GL_TEXTURE0); opengl_check;
	glBindTexture(GL_TEXTURE_2D, drawable.texture); opengl_check;
	opengl_uniform(drawable.shader, "image_texture", 0);  opengl_check;

	// Call draw function
	assert_vcl(drawable.number_triangles > 0, "Try to draw mesh_drawable with 0 triangles"); opengl_check;
	glBindVertexArray(drawable.vao);   opengl_check;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawable.vbo.at("index")); opengl_check;
	glDrawElements(GL_TRIANGLES, GLsizei(drawable.number_triangles * 3), GL_UNSIGNED_INT, nullptr); opengl_check;

	// Clean buffers
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

// To get occlusion values of drawable mesh
template <typename SCENE>
float query_occlusion(mesh_drawable const& drawable, SCENE const& scene)
{
	// Setup shader
	assert_vcl(drawable.shader != 0, "Try to draw mesh_drawable without shader");
	glUseProgram(drawable.shader); opengl_check;

	// Send uniforms for this shader
	opengl_uniform(drawable.shader, scene);
	//opengl_uniform(drawable.shader, drawable.shading, false);
	opengl_uniform(drawable.shader, "model", drawable.transform.matrix());


	// Call draw function
	assert_vcl(drawable.number_triangles > 0, "Try to draw mesh_drawable with 0 triangles"); opengl_check;
	glBindVertexArray(drawable.vao);   opengl_check;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawable.vbo.at("index")); opengl_check;

	glColorMask(0, 0, 0, 0);
	glDepthMask(GL_FALSE);

	glBeginQuery(GL_SAMPLES_PASSED, queryid1); opengl_check;
	glDisable(GL_DEPTH_TEST); opengl_check;
	glDrawElements(GL_TRIANGLES, GLsizei(drawable.number_triangles * 3), GL_UNSIGNED_INT, nullptr); opengl_check;
	glEnable(GL_DEPTH_TEST);
	glEndQuery(GL_SAMPLES_PASSED); opengl_check;

	glBeginQuery(GL_SAMPLES_PASSED, queryid2);
	glDrawElements(GL_TRIANGLES, GLsizei(drawable.number_triangles * 3), GL_UNSIGNED_INT, nullptr); opengl_check;
	glEndQuery(GL_SAMPLES_PASSED); opengl_check;


	glDepthMask(GL_TRUE);
	glColorMask(1, 1, 1, 1);

	int total_passed = 0; int total_seen = 0;

	glGetQueryObjectiv(queryid1, GL_QUERY_RESULT, &total_passed); opengl_check;
	glGetQueryObjectiv(queryid2, GL_QUERY_RESULT, &total_seen); opengl_check;


	float occ = 0;
	if (total_passed != 0)
		occ = (float)total_seen / (float)total_passed;

	// Clean buffers
	glBindVertexArray(0);

	return occ;
}


// For saturn's rings
template <typename SCENE>
void drawsatring(mesh_drawable const& drawable, SCENE const& scene, double satrad, vcl::vec3 satpos)
{
	// Setup shader
	assert_vcl(drawable.shader != 0, "Try to draw mesh_drawable without shader");
	glUseProgram(drawable.shader); opengl_check;

	// Send uniforms for this shader
	opengl_uniform(drawable.shader, scene);
	opengl_uniform(drawable.shader, "model", drawable.transform.matrix());
	opengl_uniform(drawable.shader, "sat_radius", (float)satrad, false);
	opengl_uniform(drawable.shader, "sat_pos", satpos);


	glActiveTexture(GL_TEXTURE0); opengl_check;
	glBindTexture(GL_TEXTURE_2D, drawable.texture); opengl_check;
	opengl_uniform(drawable.shader, "image_texture", 0);  opengl_check;

	// Call draw function
	assert_vcl(drawable.number_triangles > 0, "Try to draw mesh_drawable with 0 triangles"); opengl_check;
	glBindVertexArray(drawable.vao);   opengl_check;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawable.vbo.at("index")); opengl_check;
	glDrawElements(GL_TRIANGLES, GLsizei(drawable.number_triangles * 3), GL_UNSIGNED_INT, nullptr); opengl_check;

	// Clean buffers
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

#endif // DRAW_HELPER_H

