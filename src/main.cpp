#include "vcl/vcl.hpp"
#include <iostream>
#include <list>
#include <chrono>
#include <string>
#include <fstream>

//#include "Simulator.h"
#include "scene_initializer.hpp"
#include "orbit_object_helper.hpp"

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


scene_environment scene;


void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void window_size_callback(GLFWwindow* window, int width, int height);
void key_event_callback(GLFWwindow* window, int key, int scan_code, int action, int mod);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void mouvement_callback(GLFWwindow* window, double dt);

void initialize_data();
void display_interface();
void display_frame();
void cleanup();

timer_event_periodic timer(0.6f);

mesh_drawable sunbillboard;
mesh_drawable pointer_billboard;

vcl::timer_basic frame_timer;
vcl::timer_basic just_for_time;

Object_Drawable* selected = nullptr;


void switch_center_object(Object_Drawable* ob, double t) {

	if (ob != selected) {
		selected = ob;
		scene.camera.set_distance_to_center(vcl::norm(scene.camera.position() - selected->position(t)));
	}
}

Belt belt;

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
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	std::cout<<"Initialize data ..."<<std::endl;
	initialize_data();

	std::cout<<"Start animation loop ..."<<std::endl;
	user.fps_record.start();
	frame_timer.start();
	just_for_time.start();

	glGenQueries(1, &queryid1);
	glGenQueries(1, &queryid2);

	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{

		scene.light = { 0.0f, 0.0f, 0.0f };
		user.fps_record.update();
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);
		imgui_create_frame();
		if(user.fps_record.event) {
			std::string const title = "VCL Display - "+str(user.fps_record.fps)+" fps";
			glfwSetWindowTitle(window, title.c_str());
		}

		ImGui::Begin("GUI",NULL,ImGuiWindowFlags_AlwaysAutoResize);
		user.cursor_on_gui = ImGui::GetIO().WantCaptureMouse;

		double dt = frame_timer.update();
		mouvement_callback(window, dt);

		belt.update_coord(dt);

		just_for_time.update();
		scene.camera.update(just_for_time.t, glfw_current_state(window));


		display_interface();
		display_frame();



		ImGui::End();
		imgui_render_frame(window);
		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	imgui_cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();

	cleanup();

	return 0;
}

void initialize_data()
{

	Scene_initializer s = Scene_initializer::getInstance();

	// https://www.solarsystemscope.com/textures/

	sunbillboard = mesh_drawable(mesh_primitive_grid({ -1.0, -1.0, 0 }, { -1.0, 1.0, 0 }, { 1.0, 1.0, 0 }, { 1.0, -1.0, 0 }, 2, 2));
	pointer_billboard = mesh_drawable(mesh_primitive_grid({ -1, -1, 0 }, { -1, 1, 0 }, { 1, 1, 0 }, { 1, -1, 0 }, 2, 2));

	belt = create_belt((s.get_object("Sun")), 20, { 1, 0, 0 }, 30, 1, 1 , 1, 2, 0.5);

	selected = belt.elements[0];


	for (auto ast : belt.elements) {

		ast->shader = s.get_shader("Mesh Shader");
		ast->texture = s.get_texture("Moon");
		ast->shading.phong.specular = 0.0f;
		ast->shading.phong.diffuse = 0.8f;

	}



}

void cleanup() {
	Scene_initializer s = Scene_initializer::getInstance();
	s.kill_initializer();
}


void display_frame()
{

	Scene_initializer s = Scene_initializer::getInstance();


	float const dt = just_for_time.update();
	double t = just_for_time.t / 2;





	scene.translate_drawing = false;
	glDisable(GL_DEPTH_TEST);
	mesh_drawable& skybox = s.get_mesh("LQ Sphere");
	skybox.shading.phong.ambient = 3.0f;
	skybox.shader = s.get_shader("Skybox Shader");
	skybox.texture = s.get_texture("Stars");
	skybox.transform = vcl::affine_rts();
	draw(skybox, scene, false);
	glEnable(GL_DEPTH_TEST);
	scene.translate_drawing = true;




	if (selected != nullptr)
	{
		scene.camera.set_center_of_rotation(selected->position(t));

	}

	//scene.camera.set_center_of_rotation(s.get_object("Earth")->position(t));

	
	s.draw(t, scene);

	mesh_drawable tester = s.get_mesh("LQ Sphere");


	sunbillboard.transform.rotate = vcl::inverse(vcl::rotation(mat3(scene.camera.matrix_view_or_only())));

	sunbillboard.transform.scale = s.get_object("Sun")->radius*1.1;
	//sunbillboard.transform.translate = -s.get_object("Sun")->radius * scene.camera.front();
	sunbillboard.shader = s.get_shader("Simple Querry");
	float occ = query_occlusion(sunbillboard, scene);

	sunbillboard.shader = s.get_shader("Sun Billboard Shader");
	sunbillboard.transform.scale = s.get_object("Sun")->radius * 4;

	drawsunflares(sunbillboard, scene, t);

	sunbillboard.transform.translate = vcl::vec3();
	sunbillboard.shader = s.get_shader("Sun Shine Shader");
	sunbillboard.transform.scale = vcl::norm(scene.camera.position()) * 13;
	sunbillboard.texture = s.get_texture("Sun Shine");



	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);
	drawsunshine(sunbillboard, scene, scene.camera.front().z, occ*3);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (selected != nullptr) {

		pointer_billboard.transform.rotate = sunbillboard.transform.rotate;
		pointer_billboard.texture = s.get_texture("Pointer");
		pointer_billboard.shader = s.get_shader("Pointer Shader");

		vec3 p = selected->position(t);
		float r = selected->radius;
		float dst = vcl::norm(scene.camera.position() - p);
		vec3 normto = (p - scene.camera.position()) / dst;

		float size = std::max(std::tan(0.7f * pi / 180.0f) * dst, 0.0f);

		pointer_billboard.transform.scale = size;

		float move = r + 10 * size / 3;

		glDisable(GL_DEPTH_TEST);

		pointer_billboard.transform.translate = p + scene.camera.up()*move;
		pointer_billboard.transform.rotate = vcl::rotation(normto, pi / 2) * pointer_billboard.transform.rotate;
		draw(pointer_billboard, scene, false);

		pointer_billboard.transform.translate = p + scene.camera.right() * move;
		pointer_billboard.transform.rotate = vcl::rotation(normto, pi / 2) * pointer_billboard.transform.rotate;
		draw(pointer_billboard, scene, false);

		pointer_billboard.transform.translate = p - scene.camera.up() * move;
		pointer_billboard.transform.rotate = vcl::rotation(normto, pi / 2) * pointer_billboard.transform.rotate;
		draw(pointer_billboard, scene, false);

		pointer_billboard.transform.translate = p - scene.camera.right() * move;
		pointer_billboard.transform.rotate = vcl::rotation(normto, pi / 2) * pointer_billboard.transform.rotate;
		draw(pointer_billboard, scene, false);

		glEnable(GL_DEPTH_TEST);

	}


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
	scene.projection = projection_perspective(50.0f*pi/180.0f, aspect, 0.01f, 10000.0f);
}


void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	vec2 const  p1 = glfw_get_mouse_cursor(window, xpos, ypos);
	vec2 const& p0 = user.mouse_prev;
	glfw_state state = glfw_current_state(window);

	just_for_time.update();

	Dual_camera& camera = scene.camera;
	if (!user.cursor_on_gui) {
		if (state.mouse_click_left && !state.key_ctrl)
			scene.camera.manipulator_rotate_trackball(p0, p1, just_for_time.t);
		if (state.mouse_click_right)
			if (camera.getMode() == camera_mode::CENTERED)
				camera.slide_distance_to_center((p1 - p0).y);
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	glfw_state state = glfw_current_state(window);
	Scene_initializer s = Scene_initializer::getInstance();

	float const dt = just_for_time.update();
	double t = just_for_time.t / 2;


	if (!user.cursor_on_gui) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && state.key_ctrl) {
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			int width, height;
			glfwGetWindowSize(window, &width, &height);



			xpos = (xpos / height - 0.5*width/height)*2;
			ypos = ((float)(height - ypos - 1) / height - 0.5) * 2;


			std::cout << "xpos : " << xpos << " " << width << std::endl;
			std::cout << "ypos : " << ypos << " " << height << std::endl;

			float view_angle = 50.0f * pi / 180.0f /2; // SHOULD BE GLOBALLY ACCESSIBLE !!
			float min_z = 0.01f; // GLOBAL TOO !

			vec3 ray = scene.camera.front() + std::tan(view_angle) * (xpos * scene.camera.right() + ypos * scene.camera.up());



			float good_angle = 5.0f * pi / 180.0f; // to be changed;
			//good_angle = 0;

			auto is_selected = s.closest_angle(vcl::normalize(ray), scene.camera.position(), t, good_angle);

			if (is_selected != nullptr) {
				switch_center_object(is_selected, t);
			}

		}
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
