#include "vcl/vcl.hpp"



enum class camera_mode {
	CENTERED, FREE
};


class Dual_camera: public vcl::camera_base {

public:

	void switch_to_centered();
	void switch_to_free();

	vcl::vec3 position() const;
	vcl::rotation orientation() const;

	camera_mode getMode() { return mode; };

	void set_center_of_rotation(vcl::vec3 cor);
	void set_distance_to_center(double dtc);
	void look_at(vcl::vec3 eye, vcl::vec3 center, vcl::vec3 up);

	void manipulator_rotate_trackball(vcl::vec2 const& p0, vcl::vec2 const& p1);


private:

	camera_mode mode;

	vcl::camera_around_center centered_camera;
	vcl::camera_head free_camera;

	vcl::camera_base* current_camera = &centered_camera;



};