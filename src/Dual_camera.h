#ifndef DUAL_CAMERA_H
#define DUAL_CAMERA_H



#include "vcl/vcl.hpp"


// This camera (child of camera_base) combines and builds upon the centered camera and the camera head

// Modes of a Dual Camera: Free is camera_head and Centered is explicit
enum class camera_mode {
	CENTERED, FREE
};


class Dual_camera: public vcl::camera_base {

public:

	// Switch seemlessly between the two when possible
	void switch_to_centered(); // Will not move the camera att all
	void switch_to_free(); // Will not move or rotate the camera at all

	vcl::vec3 position() const;
	vcl::rotation orientation() const;

	camera_mode getMode() { return mode; };

	void set_center_of_rotation(vcl::vec3 cor, float minrad);
	void set_distance_to_center(double dtc); // No scaling, just setting
	void slide_distance_to_center(double dtc); // Equivalent to centered cameras distance manipulator
	void look_at(vcl::vec3 eye, vcl::vec3 center, vcl::vec3 up);

	void manipulator_rotate_trackball(vcl::vec2 const& p0, vcl::vec2 const& p1, double t);

	void translate_position(vcl::vec3 const& p);

	vcl::mat4 matrix_view_or_only() const; // Returns the view matrix without translation information -> useful for billboards

	void update(double t, vcl::glfw_state state); // For inertia only at the moment

	// sets centered_camera's minimal radius
	void set_min_rad(float mr) { 
		min_rad = mr; 
		centered_camera.distance_to_center = std::max(centered_camera.distance_to_center, mr * 1.1f);
	}



private:

	camera_mode mode;

	vcl::camera_around_center centered_camera;
	vcl::camera_head free_camera;

	vcl::camera_base* current_camera = &centered_camera;

	vcl::vec2 dp_trackball;

	float min_rad = 0.0f;

	// Inertia for camera rotation
	float inertia = 1;
	double last_inertia = 0;


};


#endif // !DUAL_CAMERA_H