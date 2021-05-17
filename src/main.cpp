#include "vcl/vcl.hpp"
#include <iostream>
#include <list>
#include <chrono>

#include "Dual_Camera.h"


using namespace vcl;

struct gui_parameters {
	bool display_frame = true;
};

struct user_interaction_parameters {
	vec2 mouse_prev;
	timer_fps fps_record;
	mesh_drawable global_frame;
	gui_parameters gui;
	bool cursor_on_gui;
};
user_interaction_parameters user;


struct scene_environment
{
	Dual_camera camera;
	mat4 projection;
	vec3 light;
};
scene_environment scene;


void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void window_size_callback(GLFWwindow* window, int width, int height);
void key_event_callback(GLFWwindow* window, int key, int scan_code, int action, int mod);

void mouvement_callback(GLFWwindow* window, double dt);

void initialize_data();
void display_interface();
void display_frame();


mesh_drawable sphere;
mesh_drawable disc;
timer_event_periodic timer(0.6f);

vcl::timer_basic frame_timer;

int main(int, char* argv[])
{
	std::cout << "Run " << argv[0] << std::endl;

	int const width = 1280, height = 1024;
	GLFWwindow* window = create_window(width, height);
	window_size_callback(window, width, height);
	std::cout << opengl_info_display() << std::endl;;

	imgui_init(window);
	glfwSetCursorPosCallback(window, mouse_move_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetKeyCallback(window, key_event_callback);
	
	std::cout<<"Initialize data ..."<<std::endl;
	initialize_data();

	std::cout<<"Start animation loop ..."<<std::endl;
	user.fps_record.start();
	frame_timer.start();

	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{

		scene.light = scene.camera.position();
		user.fps_record.update();
		
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);
		imgui_create_frame();
		if(user.fps_record.event) {
			std::string const title = "VCL Display - "+str(user.fps_record.fps)+" fps";
			glfwSetWindowTitle(window, title.c_str());
		}

		ImGui::Begin("GUI",NULL,ImGuiWindowFlags_AlwaysAutoResize);
		user.cursor_on_gui = ImGui::GetIO().WantCaptureMouse;

		if(user.gui.display_frame) draw(user.global_frame, scene);

		display_interface();
		display_frame();


		ImGui::End();
		imgui_render_frame(window);
		glfwSwapBuffers(window);
		glfwPollEvents();

		double dt = frame_timer.update();
		mouvement_callback(window, dt);
	}

	imgui_cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void initialize_data()
{
	// Basic setups of shaders and camera
	GLuint const shader_mesh = opengl_create_shader_program(opengl_shader_preset("mesh_vertex"), opengl_shader_preset("mesh_fragment"));
	mesh_drawable::default_shader = shader_mesh;
	mesh_drawable::default_texture = opengl_texture_to_gpu(image_raw{1,1,image_color_type::rgba,{255,255,255,255}});

	user.global_frame = mesh_drawable(mesh_primitive_frame());
	scene.camera.set_distance_to_center(10.0f);
	scene.camera.look_at({3,1,2}, {0,0,0.5}, {0,0,1});


	float const r = 0.05f; // radius of the sphere
	sphere = mesh_drawable( mesh_primitive_sphere(r) );
	sphere.shading.color = {0.5f,0.5f,1.0f};
	disc = mesh_drawable( mesh_primitive_disc(2.0f) );
	disc.transform.translate = {0,0,-r};
	

}


void display_frame()
{

	draw(disc, scene);
	float const dt = timer.update();

	// Display particles
    /*for(particle_structure& particle : particles)
    {
        sphere.transform.translate = particle.p;
        draw(sphere, scene);
    }*/

}





void display_interface()
{
	ImGui::Checkbox("Frame", &user.gui.display_frame);
	ImGui::SliderFloat("Scale", &timer.scale, 0.0f, 3.0f, "%.3f", 2.0f);
}


void window_size_callback(GLFWwindow* , int width, int height)
{
	glViewport(0, 0, width, height);
	float const aspect = width / static_cast<float>(height);
	scene.projection = projection_perspective(50.0f*pi/180.0f, aspect, 0.1f, 100.0f);
}


void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	vec2 const  p1 = glfw_get_mouse_cursor(window, xpos, ypos);
	vec2 const& p0 = user.mouse_prev;
	glfw_state state = glfw_current_state(window);



	Dual_camera& camera = scene.camera;
	if (!user.cursor_on_gui) {
		if (state.mouse_click_left && !state.key_ctrl)
			scene.camera.manipulator_rotate_trackball(p0, p1);
		if (state.mouse_click_right)
			if (camera.getMode() == camera_mode::CENTERED)
				camera.set_distance_to_center((p1 - p0).y);
	}



	user.mouse_prev = p1;
}

void key_event_callback(GLFWwindow* window, int key, int scan_code, int action, int mod) {
	glfw_state state = glfw_current_state(window);
	Dual_camera& camera = scene.camera;

	switch (key) {

	case GLFW_KEY_F:
		if (action == GLFW_PRESS) camera.switch_to_free();
		break;

	case GLFW_KEY_C:
		if (action == GLFW_PRESS) camera.switch_to_centered();
		break;

	}

}

void mouvement_callback(GLFWwindow* window, double dt) {

	float speed = 3.0f;

	if (glfwGetKey(window, GLFW_KEY_W)) { // SORRY SORRY SORRY THIS IS QWERTY because glfwGetKey only takes physical key codes. Needs to be reimplemented from scratch
		scene.camera.translate_position(scene.camera.front()*dt*speed);
	}

	if (glfwGetKey(window, GLFW_KEY_S)) {
		scene.camera.translate_position(-scene.camera.front() * dt * speed);
	}

	if (glfwGetKey(window, GLFW_KEY_A)) {
		scene.camera.translate_position(-scene.camera.right() * dt * speed);
	}

	if (glfwGetKey(window, GLFW_KEY_D)) {
		scene.camera.translate_position(scene.camera.right() * dt * speed);
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE)) {
		scene.camera.translate_position(scene.camera.up() * dt * speed);
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
		scene.camera.translate_position(-scene.camera.up() * dt * speed);
	}
}

void opengl_uniform(GLuint shader, scene_environment const& current_scene)
{
	opengl_uniform(shader, "projection", current_scene.projection);
	opengl_uniform(shader, "view", current_scene.camera.matrix_view());
	opengl_uniform(shader, "light", current_scene.light, false);
}



