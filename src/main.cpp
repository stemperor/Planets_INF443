#include "vcl/vcl.hpp"
#include <iostream>
#include <list>
#include <chrono>
#include <string>
#include <fstream>
#include <queue>

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
mesh_drawable saturn_billboard;

vcl::timer_basic frame_timer;
vcl::timer_basic just_for_time;

Object_Drawable* selected = nullptr;
Object_Drawable* focused = nullptr;

float avg_occ = -1;
float occ_factor = 1.0f;
std::queue<float> avg_occs;
int avg_times = 10;

float aspect;

float near_field_min = 0.0101;
float near_field_max = 0.07;
float near_field_max_dist = 5000.0f;


void switch_center_object(Object_Drawable* ob, double t) {

	if (ob != selected) {
		selected = ob;
	}
}

void focus_on_selected(double t) {
	if (selected != nullptr) {
		focused = selected;
		scene.camera.set_distance_to_center(vcl::norm(scene.camera.position() - focused->position(t)));
	}
}

Belt belt;

int main(int, char* argv[])
{
	std::cout << "Run " << argv[0] << std::endl;

	int const width = 1280, height = 1024;
	GLFWwindow* window = create_window(width, height);
	window_size_callback(window, width, height);
	aspect = (float)width / height;
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
	glDepthFunc(GL_LESS);
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

		belt.update_coord(dt/10);

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

	sunbillboard = mesh_drawable(mesh_primitive_grid({ -2, -2, 0 }, { -2, 2, 0 }, { 2, 2, 0 }, { 2, -2, 0 }, 2, 2));
	pointer_billboard = mesh_drawable(mesh_primitive_grid({ -1.0, -1.0, 0 }, { -1.0, 1.0, 0 }, { 1.0, 1.0, 0 }, { 1.0, -1.0, 0 }, 2, 2));
	saturn_billboard = mesh_drawable(mesh_primitive_grid({ -1, 0, -1 }, { -1, 0, 1 }, { 1, 0, 1 }, {1, 0, -1}, 1, 1));



	belt = create_belt((s.get_object("Sun")), 200000, { 1, 0, 0 }, 200, 10, 100 , 1, 2, 10);

	focused = s.get_object("Sun");

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




	if (focused != nullptr)
	{
		scene.camera.set_center_of_rotation(focused->position(t), focused->radius_drawn());

	}

	//scene.camera.set_center_of_rotation(s.get_object("Earth")->position(t));

	
	s.draw(t, scene);

	mesh_drawable tester = s.get_mesh("LLQ Sphere");


	sunbillboard.transform.rotate = vcl::inverse(vcl::rotation(mat3(scene.camera.matrix_view_or_only())));



	sunbillboard.transform.scale = s.get_object("Sun")->radius;
	sunbillboard.transform.translate = s.get_object("Sun")->radius * vcl::normalize(scene.camera.position());
	sunbillboard.shader = s.get_shader("Simple Querry");
	float occ = query_occlusion(sunbillboard, scene);


	// Makeshift way to prevent unexplained occlusion testing flickering...: average over avg_times frames
	if (avg_occs.size() < avg_times) {

		avg_occ = avg_occ * avg_occs.size() / (avg_occs.size() + 1);
	}
	else {
		avg_occ -= (avg_occs.front() - occ) / avg_times;
		avg_occs.pop();
	}

	avg_occs.push(occ);
	std::cout << norm(scene.camera.position()) << std::endl;

	float dst = norm(scene.camera.position());

	float sunrad = s.get_object("Sun")->radius_drawn();

	occ = avg_occ;

	// removes lens flare when properly seeing the sun - changing fov messes with the setting (for now)
	if (dst < 15 * sunrad) {
		occ *= std::max(0.0f, (dst - 5 * sunrad) / (15 * sunrad - 5 * sunrad));
	}


	sunbillboard.shader = s.get_shader("Sun Billboard Shader");
	sunbillboard.transform.scale = s.get_object("Sun")->radius;

	//drawsunflares(sunbillboard, scene, t);

	sunbillboard.transform.translate = vcl::vec3();
	sunbillboard.shader = s.get_shader("Sun Shine Shader");
	sunbillboard.transform.scale = vcl::norm(scene.camera.position())/10 * 3;
	sunbillboard.texture = s.get_texture("Sun Shine");



	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);
	drawsunshine(sunbillboard, scene, scene.camera.front().z, occ*occ_factor);
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
	ImGui::SliderFloat("planet_size", &p_size, 1.0f, 10.0f, "%.3f", 4.0f);
	ImGui::SliderFloat("sun brightness", &occ_factor, 0.0f, 1.0f, "%.3f", 1.0f);

	if (selected != nullptr) {

		if (selected->parent != nullptr) {
			if (ImGui::Button(("<-- " + selected->parent->name).c_str())) {
				selected = selected->parent;
			}
		}

		ImGui::Button((selected->name).c_str());

		for (auto child : selected->enfants){
			if (child->name.substr(0, 3) != "ast" || child->name == "ast0") {
				if (ImGui::Button(("\t" + child->name).c_str())) {
					selected = child;

				}

			}
		}


	}

}


void window_size_callback(GLFWwindow* , int width, int height)
{
	glViewport(0, 0, width, height);
	float const aspect_ = width / static_cast<float>(height);

	aspect = aspect_;
	scene.projection = projection_perspective(50.0f*pi/180.0f, aspect, 0.05f, 20000.0f);
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
	just_for_time.update();
	float t = just_for_time.t;

	switch (key) {

	case GLFW_KEY_F:
		if (action == GLFW_PRESS) camera.switch_to_free();
		break;

	case GLFW_KEY_C:
		if (action == GLFW_PRESS) camera.switch_to_centered();
		break;
	
	case GLFW_KEY_G:
		focus_on_selected(t);
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



			float view_angle = 50.0f * pi / 180.0f /2; // SHOULD BE GLOBALLY ACCESSIBLE !!
			float min_z = 0.01f; // GLOBAL TOO !

			vec3 ray = scene.camera.front() + std::tan(view_angle) * (xpos * scene.camera.right() + ypos * scene.camera.up());



			float good_angle = 1.0f * pi / 180.0f; // to be changed;
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

	float near_field = near_field_min + (near_field_max - near_field_min) * std::min(1.0f, norm(scene.camera.position()) / near_field_max_dist);
	scene.projection = projection_perspective(50.0f * pi / 180.0f, aspect, near_field, 20000.0f);


}
