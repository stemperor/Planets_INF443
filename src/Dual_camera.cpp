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

	// Allows for no visual change when leaving centered mode
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

void Dual_camera::set_center_of_rotation(vcl::vec3 cor, float minrad)
{
	min_rad = minrad;
	centered_camera.distance_to_center = std::max(centered_camera.distance_to_center, minrad + 0.1f); // Make sure planets are never entered (does not work well with asteroids)
	centered_camera.center_of_rotation = cor;

}

void Dual_camera::slide_distance_to_center(double dtc)
{
	if (mode == camera_mode::CENTERED) {

		// Make sure planets are never entered (does not work well with asteroids)
		centered_camera.distance_to_center = min_rad + (centered_camera.distance_to_center - min_rad)* (1.0f + dtc);
		centered_camera.distance_to_center = std::max(centered_camera.distance_to_center, 0.1f);
	}
		centered_camera.manipulator_scale_distance_to_center(dtc);
}

void Dual_camera::set_distance_to_center(double dtc)
{
	centered_camera.distance_to_center = dtc;
}

void Dual_camera::look_at(vcl::vec3 eye, vcl::vec3 center, vcl::vec3 up)
{
	centered_camera.look_at(eye, center, up);
	free_camera.position_camera = centered_camera.position();
	free_camera.orientation_camera = centered_camera.orientation();

}

void Dual_camera::manipulator_rotate_trackball(vcl::vec2 const& p0, vcl::vec2 const& p1, double t)
{
	last_inertia = t;
	if (mode == camera_mode::CENTERED) {
		centered_camera.manipulator_rotate_trackball(p0, p1);
	}
	else {
		vcl::rotation const r = trackball_rotation(p0, p1);
		free_camera.orientation_camera = free_camera.orientation_camera * inverse(r);
	}

	// Stops inertia after a while
	if (vcl::norm(p1 - p0) > 0.01)
		dp_trackball = p1 - p0;
}

void Dual_camera::translate_position(vcl::vec3 const& p)
{
	if (mode == camera_mode::FREE) {
		free_camera.position_camera += p;
	}
}

vcl::mat4 Dual_camera::matrix_view_or_only() const
{
	return vcl::inverse(vcl::frame(orientation(), { 0.0f, 0.0f, 0.0f })).matrix();
}

void Dual_camera::update(double t, vcl::glfw_state state)
{

	// Inertia is calculated at each moment and when the mouse button is let go, rotation speed decreases exponentially

	// This does not work too well because it doesn't always store the last movement 
	//(if there is no mouvement, manipulator_rotate_trackball is never called and intertia never set to zero)
	// Easily fixed but tedious 

	if (!vcl::is_equal(dp_trackball, { 0.0f, 0.0f })) {
		dp_trackball *= std::exp(-(t - last_inertia)/inertia);

		if (vcl::norm(dp_trackball) < 0.00001)
			dp_trackball = { 0.0f, 0.0f };

		if (!state.mouse_click_left)
			manipulator_rotate_trackball({ 0.0f, 0.0f }, dp_trackball, t);
	}


}

