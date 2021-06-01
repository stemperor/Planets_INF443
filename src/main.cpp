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

// Called at each loop to check whether any keys have been pressed
void mouvement_callback(GLFWwindow* window, double dt);

void initialize_data();
void display_interface();
void display_frame();
void cleanup();

// Useful billboards for visual effects
mesh_drawable sunbillboard; // Sun flares and lens flares
mesh_drawable pointer_billboard; // Selection indicator
mesh_drawable saturn_billboard; // Saturn's rings

// Various timers with various uses
timer_event_periodic timer(0.6f);
vcl::timer_basic frame_timer;
vcl::timer_basic just_for_time;


Object_Drawable* selected = nullptr; // Currently selected object: marked with red indicators
Object_Drawable* focused = nullptr; // Object in the center of the camera in centered_camera mode;

float avg_occ = -1; // Occlusion of the sun over the past avg_times frames
float occ_factor = 1.0f; // Multiplies occlusion to change lens flare intensity
std::queue<float> avg_occs; // Helps calculate average sun occlusion -> avoids flickering
int avg_times = 20;

// Store window aspect ratio
float aspect;


// Set selected object
void switch_center_object(Object_Drawable* ob, double t) {

	if (ob != selected) {
		selected = ob;
	}
}

// Choose selected object as object at the center of the camera
void focus_on_selected(double t) {
	if (selected != nullptr) {
		focused = selected;
		scene.camera.set_distance_to_center(vcl::norm(scene.camera.position() - focused->position(t))); // Distance to center updated to avoid 
		scene.camera.look_at(scene.camera.position(), focused->position(t), scene.camera.up());
	}
}

// Asteroid belt object
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

	// Allows for transparent billboards
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	std::cout<<"Initialize data ..."<<std::endl;
	initialize_data();

	std::cout<<"Start animation loop ..."<<std::endl;
	user.fps_record.start();
	frame_timer.start();
	just_for_time.start();

	// Create OpenGl Querries (see query occlusion in draw_helper.hpp)
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

		// As opposed to other planets, belt asteroid positions are not defined at every instant and have to be incrementally calculated
		// the dt/10 factor is arbitrary - They evolve on a different timescale than planets
		belt.update_coord(dt/10);

		// Update camera. Dual_Camera object has a partial implementation of inertia (at least rotational) - See Dual_Camera for more info
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
	// Retrieve (and create) the scene manager (with a poorly chosen name). It contains all info (Meshes, Shaders, Planets, Textures) that we had time to remove from main.cpp
	// See Scene_initializer.hpp
	Scene_initializer s = Scene_initializer::getInstance();

	// Create the billboard. Sizes are chosen arbitrarily to fit the current planet size. Some can be changed, some not.
	sunbillboard = mesh_drawable(mesh_primitive_grid({ -2, -2, 0 }, { -2, 2, 0 }, { 2, 2, 0 }, { 2, -2, 0 }, 2, 2));
	pointer_billboard = mesh_drawable(mesh_primitive_grid({ -1.0, -1.0, 0 }, { -1.0, 1.0, 0 }, { 1.0, 1.0, 0 }, { 1.0, -1.0, 0 }, 2, 2));
	saturn_billboard = mesh_drawable(mesh_primitive_grid({ -1, 0, -1 }, { -1, 0, 1 }, { 1, 0, 1 }, {1, 0, -1}, 2, 2));

	saturn_billboard.texture = s.get_texture("Saturn Rings");
	saturn_billboard.shader = s.get_shader("Satring Shader");

	// Creates our asteroid belt. See Orbit_object.hpp and Scene_initializer.hpp
	belt = create_belt((s.get_object("Sun")), 200000, { 1, 0, 0 }, 200, 10, 100 , 1, 2, 10);

	just_for_time.update();
	selected = s.get_object("Saturn");
	
	focus_on_selected(just_for_time.t);

	scene.camera.set_distance_to_center(s.get_object("Saturn")->radius_drawn() * 2);


	for (auto ast : belt.elements) {

		ast->shader = s.get_shader("Mesh Shader");
		ast->texture = s.get_texture("Moon");
		ast->shading.phong.specular = 0.0f;
		ast->shading.phong.diffuse = 0.8f;

	}



}

void cleanup() {
	Scene_initializer s = Scene_initializer::getInstance();
	s.kill_initializer(); // Singleton destroyer
}


void display_frame()
{
	Scene_initializer s = Scene_initializer::getInstance();


	float const dt = just_for_time.update();
	double t = just_for_time.t / 2;


	// Draw the skybox
	scene.translate_drawing = false; // Parameter added to scene to know whether an object should be translated with the camera
	// Note: Could be better implemented with a special draw function. But no more time.
	glDisable(GL_DEPTH_TEST); // Drawn first without depth test: make sure it is behind everything
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


	
	s.draw(t, scene);

	vec3 satpos = s.get_object("Saturn")->position(t);
	float satrad = s.get_object("Saturn")->radius_drawn();

	saturn_billboard.transform.translate = satpos;
	saturn_billboard.transform.scale = satrad * 2.2;
	saturn_billboard.transform.rotate = vcl::rotation(vec3(1, 1, 0), vcl::pi / 10); // See saturn creation in Scene_initializer. Better design needed

	drawsatring(saturn_billboard, scene, satrad, satpos);


	// Applied to all billboard that must follow the camera
	sunbillboard.transform.rotate = vcl::inverse(vcl::rotation(mat3(scene.camera.matrix_view_or_only())));


	
	/* Right after drawing all physical objects, sun occlusion query is performed.
	* 
	* A square billboard is placed in front of the sun in the direction of the camera.
	* This is required since the sun has already been drawn and would interfere.
	* A billboard is used since we have found the occlusion querries work poorly with high vertex numbers and the results are quite good as is
	*/
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


	float dst = norm(scene.camera.position());

	float sunrad = s.get_object("Sun")->radius_drawn();

	occ = avg_occ;

	// removes lens flare when close to the sun - changing fov messes with the setting (for now)
	if (dst < 30 * sunrad) {
		occ *= std::max(0.0f, (dst - 5 * sunrad) / (30 * sunrad - 5 * sunrad));
	}


	// Draw solar winds and such
	sunbillboard.transform.translate = vcl::vec3();
	sunbillboard.shader = s.get_shader("Sun Billboard Shader");
	sunbillboard.transform.scale = s.get_object("Sun")->radius*5; // The *5 doesn't change anything except avoid us seeing the edge of the billboard

	drawsun(sunbillboard, scene, t);

	
	// Draw lens flare
	sunbillboard.shader = s.get_shader("Sun Shine Shader");
	sunbillboard.transform.scale = vcl::norm(scene.camera.position())/10 * 5; // Impacts the size of the flare on the screen (constant)
	sunbillboard.texture = s.get_texture("Sun Shine");


	glBlendFunc(GL_ONE, GL_ONE); // Additive blending for best results
	glDisable(GL_DEPTH_TEST); // In front of everything
	drawsunshine(sunbillboard, scene, scene.camera.front().z, occ*occ_factor);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// Draw the selection marker
	if (selected != nullptr) {

		pointer_billboard.transform.rotate = sunbillboard.transform.rotate;
		pointer_billboard.texture = s.get_texture("Pointer");
		pointer_billboard.shader = s.get_shader("Pointer Shader");

		/* Explanation without entering in the code's detail
		* 
		* The marker is to be seen through every object and any visual effect. Must follow the planet.
		* The markers themselves must remain the same size but always surround the planet.
		* They are thus scaled with distance and the planet radius.
		* 
		* The marker is just a triangle and will be drawn 4 times (up, down, left, right)
		*  Orientation depends on the camera
		*/

		vec3 p = selected->position(t);
		float r = selected->radius_drawn();
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
	ImGui::SliderFloat("sun brightness", &occ_factor, 0.0f, 2.0f, "%.3f", 1.0f);

	// The following helps explore the planete tree
	// First is the parent button, then the planet button, then the children buttons
	// Clicking selects them

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
	scene.projection = projection_perspective(50.0f*pi/180.0f, aspect, 0.1f, 20000.0f); // Far field was increased - we are very close to depth buffer resolution limit!
}


void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	vec2 const  p1 = glfw_get_mouse_cursor(window, xpos, ypos);
	vec2 const& p0 = user.mouse_prev;
	glfw_state state = glfw_current_state(window);

	just_for_time.update();

	// See Dual camera object
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
	
	case GLFW_KEY_L:
		focus_on_selected(t);
		break;

	case GLFW_KEY_G:
		focus_on_selected(t);
		if (selected != nullptr) {
			scene.camera.set_distance_to_center(focused->radius_drawn()*4);
		}
		

	}

}

// Function to select objects on screen
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

			// The next four lines calculate the ray vector going from the camera to the point on screen that was clicked

			xpos = (xpos / height - 0.5*width/height)*2;
			ypos = ((float)(height - ypos - 1) / height - 0.5) * 2;

			float view_angle = 50.0f * pi / 180.0f /2; // = fov. Should be global but no time.
			vec3 ray = scene.camera.front() + std::tan(view_angle) * (xpos * scene.camera.right() + ypos * scene.camera.up());

			/* Object selection works as follows:
			* 
			* - A maximum angle is given (good_angle), here equal to one degree.
			* - An object is selected if it is the closest that fills on of the two following conditions:
			* -		The object itself was clicked on (the sphere of radius r surrounding the object)
			* -		The angle between the object and the ray is smaller than good_angle
			*/

			float good_angle = 1.0f * pi / 180.0f;

			auto is_selected = s.closest_angle(vcl::normalize(ray), scene.camera.position(), t, good_angle);


			// If nothing is selected, selected is set to nullptr
			switch_center_object(is_selected, t);
			

		}
	}
		
}

void mouvement_callback(GLFWwindow* window, double dt) {

	float speed = 3.0f;

	if (glfwGetKey(window, GLFW_KEY_W)) { // SORRY this is qwerty because glfwGetKey only takes physical key codes. Needs to be reimplemented from scratch to work with AZERTY
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
