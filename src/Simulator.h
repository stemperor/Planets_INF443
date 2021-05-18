#include "vcl/vcl.hpp"
#include <map>
#include <string>
#include <stdexcept>


struct Mass_object {

	float mass;

	vcl::vec3 position;
	vcl::vec3 axis;

	vcl::vec3 speed;
	vcl::vec3 last_move;

	int attraction_level; // A mass object will only attract objects of lower "or equal" attraction levels (use to exclude tiny or giant objects)
	bool attracts_similar = true; // removes the "or equal" condition of attraction_level

	double potential_energy; // Let's keep things realistic
	double total_energy; // For manual use only.

	void update_energy() {
		total_energy = 0.5 * std::pow(vcl::norm(speed), 2) * mass + potential_energy;
	}

};

const double G = 1; // Ignoring scale for now : all unit arbitrary;

class Simulator { // Uses pointers  FOR NOW

public:

	void add_object(std::string name, Mass_object * object) {
		if (objects.count(name) != 0)
			throw std::invalid_argument("An object with that name is already registered.");

		// Update potential energy
		object->potential_energy = 0;

		for (auto obj : objects) {
			potential_to(*obj.second, *object);
			potential_to(*object, *obj.second);
			obj.second->update_energy();
		}
		
		objects[name] = object;
		object->update_energy();
	}

	void remove_object(std::string name) {
		objects.erase(name);
	}

	void simulate(double timestep) {

		for (auto obj1 : objects) {

			vcl::vec3 delta_speed;

			for (auto obj2 : objects) {
				// Here we look at the effect of obj2 on obj1 (there may be more efficient ways)
				
				if (obj1.first == obj2.first)
					continue;

				// Update potential energy
				obj1.second->potential_energy = 0;
				double force = potential_to(*obj2.second, *obj1.second);

				if (force != -1.0) { // This tells us that obj2 attracts obj1
					delta_speed += force * (obj2.second->position - obj1.second->position)*timestep;
					
				}

			}


			// Now we force conserved total energy

			vcl::vec3 last_move = obj1.second->last_move;
			obj1.second->last_move = delta_speed;
			obj1.second->speed += delta_speed;

			if (obj1.second->total_energy - obj1.second->potential_energy < 0) {
				std::cout << "energy gain: " << obj1.second->potential_energy - obj1.second->total_energy << std::endl;
				obj1.second->total_energy = obj1.second->potential_energy;
				continue;
			}


			double last_speed_norm2 = std::pow(vcl::norm(last_move), 2);

			if (last_speed_norm2 == 0) continue;

			double speed2 = std::pow(vcl::norm(obj1.second->speed), 2);
			double expected_speed = std::sqrt(2 * (obj1.second->total_energy - obj1.second->potential_energy) / obj1.second->mass);
			double angle = vcl::dot(last_move, obj1.second->speed);

			double A = last_speed_norm2;
			double B = -2 * angle;
			double C = speed2 - expected_speed * expected_speed;

			double delta = B * B - 4 * A * C;

			double lambda = 0;

			if (delta >= 0) {
				if (B > 0)
					lambda = (-B + std::sqrt(delta)) / 2 / A;
				else
					lambda = (-B - std::sqrt(delta)) / 2 / A;
			}
			else {
				lambda = -B / 2 / A;
			}
			
			obj1.second->speed -= lambda * last_move;

			std::cout << "delta1 " << delta << std::endl;

			// Avec la deuxième direction aussi (overkill au possible)

			last_speed_norm2 = std::pow(vcl::norm(delta_speed), 2);

			if (last_speed_norm2 == 0) continue;

			speed2 = std::pow(vcl::norm(obj1.second->speed), 2);
			expected_speed = std::sqrt(2 * (obj1.second->total_energy - obj1.second->potential_energy) / obj1.second->mass);
			angle = vcl::dot(delta_speed, obj1.second->speed);

			A = last_speed_norm2;
			B = -2 * angle;
			C = speed2 - expected_speed * expected_speed;

			delta = B * B - 4 * A * C;

			lambda = 0;

			if (delta >= 0) {
				lambda = std::min((-B + std::sqrt(delta)) / 2 / A, (-B - std::sqrt(delta)) / 2 / A);
			}
			else {
				lambda = -B / 2 / A;
			}

			obj1.second->speed -= lambda * delta_speed;

			std::cout << "delta2 " << delta << std::endl;

			std::cout << vcl::norm(obj1.second->speed) - expected_speed << std::endl;

		}

		for (auto obj : objects) {

			//std::cout << obj.first << " pos " << obj.second->position << std::endl;
			//std::cout << obj.first << " " << obj.second->speed << std::endl;
			obj.second->position += obj.second->speed * timestep; // update positions
		}
	}

	void simulate(double time, double timestep) {

		int n_timesteps = (int)time / timestep;

		for (int i = 0; i < n_timesteps; i++)
			simulate(timestep);

		simulate(timestep * n_timesteps - time);

	}

private:
	std::map<std::string, Mass_object*> objects;

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