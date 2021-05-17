#include "Dual_camera.h"

void Dual_camera::switch_to_centered()
{
	if (mode == camera_mode::CENTERED) return;

	centered_camera.look_at(free_camera.position(), centered_camera.center_of_rotation, free_camera.up());

	current_camera = &centered_camera;

	mode = camera_mode::CENTERED;

}

void Dual_camera::switch_to_free()
{
	if (mode == camera_mode::FREE) return;

	free_camera.position_camera = centered_camera.position();
	free_camera.orientation_camera = centered_camera.orientation();

	current_camera = &free_camera;

	mode = camera_mode::FREE;

}

vcl::vec3 Dual_camera::position() const
{
	return current_camera->position();
}

vcl::rotation Dual_camera::orientation() const
{
	return current_camera->orientation();
}

void Dual_camera::set_center_of_rotation(vcl::vec3 cor)
{
	centered_camera.center_of_rotation = cor;
}

void Dual_camera::set_distance_to_center(double dtc)
{
	if (mode == camera_mode::CENTERED)
		centered_camera.manipulator_scale_distance_to_center(dtc);
}

void Dual_camera::look_at(vcl::vec3 eye, vcl::vec3 center, vcl::vec3 up)
{
	centered_camera.look_at(eye, center, up);
	free_camera.position_camera = centered_camera.position();
	free_camera.orientation_camera = centered_camera.orientation();

}

void Dual_camera::manipulator_rotate_trackball(vcl::vec2 const& p0, vcl::vec2 const& p1)
{
	if (mode == camera_mode::CENTERED) {
		centered_camera.manipulator_rotate_trackball(p0, p1);
	}
	else {
		vcl::rotation const r = trackball_rotation(p0, p1);
		free_camera.orientation_camera = free_camera.orientation_camera * inverse(r);
	}
}

void Dual_camera::translate_position(vcl::vec3 const& p)
{
	if (mode == camera_mode::FREE) {
		free_camera.position_camera += p;
	}
}

