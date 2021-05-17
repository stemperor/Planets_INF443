#include "vcl/vcl.hpp"
#include <map>
#include <string>
#include <stdexcept>


struct Mass_object {

	float mass;

	vcl::vec3 position;
	vcl::vec3 axis;

	vcl::vec3 speed;

	int attraction_level; // A mass object will only attract objects of lower "or equal" attraction levels (use to exclude tiny or giant objects)
	bool attracts_similar; // removes the "or equal" condition of attraction_level

	double potential_energy; // Let's keep things realistic
	double total_energy; // For manual use only.

	void update_energy() {
		total_energy = 0.5 * std::pow(vcl::norm(speed), 2) * mass + potential_energy;
	}

};

const double G = 1; // Ignoring scale for now : all unit arbitrary;

class Simulator {

public:

	void add_object(std::string name, Mass_object & object) {
		if (objects.count(name) != 0)
			throw std::invalid_argument("An object with that name is already registered.");

		// Update potential energy
		object.potential_energy = 0;

		for (auto obj : objects) {
			potential_to(obj.second, object);
			potential_to(object, obj.second);
			obj.second.update_energy();
		}
		
		objects[name] = object;
		object.update_energy();
	}

	void remove_object(std::string name) {
		objects.erase(name);
	}

	void simulate(double timestep) {

		for (auto obj : objects) {
			obj.second.position += obj.second.speed*timestep; // update positions
		}

		for (auto obj1 : objects) {

			for (auto obj2 : objects) {
				// Here we look at the effect of obj2 on obj1 (there may be more efficient ways)
				
				// Update potential energy
				double force = potential_to(obj2.second, obj1.second);

				if (force != -1.0) { // This tells us that obj2 attracts obj1
					obj1.second.speed += force * (obj2.second.position - obj1.second.position)*timestep;
				}

			}


			// Now we force conserved total energy
			double expected_speed = std::sqrt(2 * (obj1.second.total_energy - obj1.second.potential_energy) / obj1.second.mass);
			double speed = vcl::norm(obj1.second.speed);
			
			if (speed != 0) // Eh couldn't bother we'll do it next time
				obj1.second.speed *= expected_speed / speed;
			
		}
	}

	void simulate(double time, double timestep) {

		int n_timesteps = (int)time / timestep;

	}

private:
	std::map<std::string, Mass_object&> objects;

	// Calculates the potential energy of To in From's field
	// Return a useful value
	static double potential_to(Mass_object& from, Mass_object& to) {
		

		if (from.attraction_level > to.attraction_level || (from.attraction_level == to.attraction_level && from.attracts_similar)) // Checks whether the impact of From on To should be simulated
		{
			double distance = vcl::norm(from.position - to.position);
			if (distance == 0)
				throw std::runtime_error("Two objects have the same position == BOOM..."); // Berk. Custom exception WIP
			double PE = -G * from.mass * to.mass / distance;
			to.potential_energy += PE;

			return -PE / (distance * distance);

		}
		return -1.0;
	}

};