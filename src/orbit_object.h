#ifndef ORBIT_OBJECT_H
#define ORBIT_OBJECT_H
#include "vcl/vcl.hpp"
#include <stdexcept>
#include "draw_helper.hpp"


float G = 1;

void print(float s, std::string mess = "") {
    std::cout << mess << ": " << s << std::endl;
};

float p_size = 1.0f; // Planet sizes -> use to scale all objects except asteroids and the sun

// All planets are initially set at the same axis. 
// A large start time randomizes their positions 
double random_rotate_time = 100000.0; 


// Characterizes a circular orbit, the caracteristics of which can be gotten at any time t.
struct Orbit_Object {

    // Caractéristiques de l'objet
    float mass ;

    // Caractéristiques de la trajectoire
    float period ; // = sqrt(4*pow(3.14,2)*pow(radius_orbit, 3)/(G*mass_parent));
    vcl::vec3 axis ; // normalisé à 1 (utiliser normalize())
    float radius_orbit ;

    vcl::vec3 diameter_ini ; // rayon initial, à entrer par l'utilisateur, normalisé à 1 (utiliser normalize())

    vcl::vec3 position(float t){        // quel type pour le temps ?
        t += random_rotate_time;
        if (norm(diameter_ini) == 0 || norm(axis) == 0)
            {vcl::call_error("norme non nulle", "définir un vecteur non vide", "Orbit_Object", "position", 22);}
        float angle = 2*3.14*t/period ;
        vcl::vec3 axis1 = diameter_ini;
        vcl::vec3 axis3 = axis;
        vcl::vec3 axis2 = cross(axis3, axis1);
        return (std::cos(angle)*radius_orbit*axis1 + std::sin(angle)*radius_orbit*axis2);
    }

    vcl::vec3 speed(float t){
        t += random_rotate_time;
        if (norm(diameter_ini) == 0 || norm(axis) == 0)
            {vcl::call_error("norme non nulle", "définir un vecteur non vide", "Orbit_Object", "position", 22);}
        float angle = 2*3.14*t/period ;
        vcl::vec3 axis1 = diameter_ini / norm(diameter_ini);
        vcl::vec3 axis3 = axis / norm(axis);
        vcl::vec3 axis2 = cross(axis3, axis1);
        float v = 2*3.14*radius_orbit/period ;      // la norme est constante dans le cas de l'orbite circulaire
        return (std::cos(angle)*v*axis2 + std::sin(angle)*v*axis1);
     }



};

// Initializes an Obrit_Object. The trajectory is orthogonal to ax, and around zero.
void init_orbit_circ(Orbit_Object& obj, float parent_mass, vcl::vec3 initial_position, vcl::vec3 ax = { 0.0f, 0.0f, 0.0f });


// Any object physical object that will be drawn. It is part of a tree.
// This class is abstract
struct Object_Drawable {
    Object_Drawable* parent = nullptr; // nullptr means top of the tree
    std::vector<Object_Drawable*> enfants;

    std::string name;
    float radius;
    vcl::vec3 rotation_axis = vcl::vec3(1, 0, 0);
    float rotation_speed = 0.0f;

    // inital rotation to correct object axis - does not change with time
    vcl::rotation rot_corr_axis = vcl::rotation();

    float rotation_angle(float t) {
        return (rotation_speed * t);
    }

    // Must be redefined in children classes
    virtual vcl::vec3 position(double t) = 0;

    virtual vcl::vec3 position(double t, vcl::vec3 parent_pos) = 0;

    // Caractéristiques de la texture

    GLuint texture;
    GLuint shader;
    shading_parameters_phong shading;

    // A reference to a mesh is stores for reusability purposes
    mesh_drawable& mesh;

    Object_Drawable(mesh_drawable& d) :mesh(d) {}


    float virtual radius_drawn(){
        return radius;
    }

    void virtual setup_mesh(double t) {
        mesh.shader = shader;
        mesh.shading = shading;

        // another systematic rotation because textures usually needed it
        mesh.transform.rotate = vcl::rotation({ 0, 0, 1 }, -vcl::pi / 2) * rot_corr_axis;
        mesh.transform.rotate = vcl::rotation(rotation_axis, rotation_angle(t)) * mesh.transform.rotate;
        mesh.transform.translate = position(t);

        mesh.transform.scale = radius*p_size;
        mesh.texture = texture;
    }

    void virtual draw_obj(double t, scene_environment scene) {
        setup_mesh(t);
        draw(mesh, scene, true);
    };

};


// Just like Object_Drawables but with a Orbit_Object to define a trajectory
struct Planete_Drawable: public Object_Drawable {
    Orbit_Object* planete = nullptr;

    Planete_Drawable(mesh_drawable& d, vec3 initpos, vec3 axis, float parentmass) :Object_Drawable(d) {
        planete = new Orbit_Object();
        init_orbit_circ(*planete, parentmass, initpos, axis);

        shading.phong.specular = 0.0f;
        shading.phong.ambient = 0.0f;
        shading.phong.diffuse = 0.8f;
    }

    Planete_Drawable(mesh_drawable& d) : Object_Drawable(d) {

    }

    float virtual radius_drawn() {
        return radius*p_size;
    }


    virtual vcl::vec3 position(double t) {
        if (parent == nullptr)
            return { 0,0,0 };
        else
            return planete->position(t) + parent->position(t);
    }

    virtual vcl::vec3 position(double t, vcl::vec3 parent_pos) {
        if (parent == nullptr)
            return vcl::vec3();
        else
            return planete->position(t) + parent_pos;
    }

};

// Earth is special and has its own object. It is defined by:
/*
* A texture
* A cloud texture
* A specularity texture
* A bump map
* A nighttime texture
* Other paremeters to do with clouds
*/
struct Earth_Drawable : public Planete_Drawable{       //Il faudrait avoir un meshDrawable quelque part
    
   // Clouds turn with the earth at a slightly different speed for parallax results
    float cloud_rel_speed = 0.05;

    GLuint cloud_texture;

    GLuint night_texture;
    GLuint bump_texture;
    GLuint spec_texture;

    GLuint cloud_shader;

    shading_parameters_phong cloud_shading;

    double cloud_height = 0.015;

    Earth_Drawable(mesh_drawable & d, vec3 initpos, vec3 axis, float parentmass) :Planete_Drawable(d, initpos, axis, parentmass) {}

    void virtual draw_obj(double t, scene_environment scene) {
        setup_mesh(t);
        drawearth(mesh, scene, night_texture, spec_texture, bump_texture);

        mesh.transform.scale += cloud_height*p_size;

        mesh.transform.rotate = vcl::rotation(rotation_axis, cloud_rel_speed*t) * mesh.transform.rotate;
        mesh.shading = cloud_shading;
        mesh.shader = cloud_shader;
        mesh.texture = cloud_texture;
        
        draw(mesh, scene, true);

    }

};


struct Sun_Drawable : public Object_Drawable {


    Sun_Drawable(mesh_drawable& d) :Object_Drawable(d) {}

    virtual vcl::vec3 position(double t) { return { 0.0, 0.0, 0.0 }; }

    virtual vcl::vec3 position(double t, vcl::vec3 parent_pos) { return { 0.0, 0.0, 0.0 }; }

    void virtual draw_obj(double t, scene_environment scene) {

        mesh.shader = shader;

        mesh.transform.rotate = vcl::rotation(rotation_axis, rotation_angle(t));
        mesh.transform.translate = position(t);

        mesh.transform.scale = radius;

        drawsun(mesh, scene, t);
    }
};


// Asteroid drawables are a bit weird since they do not really know their position. They are updated by a belt object
struct Asteroid_Drawable : public Object_Drawable {

    float mass;
    vcl::vec3 pos;
    vcl::vec3 speed;

    Asteroid_Drawable(vcl::mesh_drawable & m):Object_Drawable(m){}

    virtual vcl::vec3 position(double t) {
        if (parent == nullptr)
            return pos;
        else
            return pos + parent->position(t);
    }

    virtual vcl::vec3 position(double t, vcl::vec3 parent_pos) {
        return pos + parent_pos;
    }

    void virtual draw_obj(double t, scene_environment scene) {
        setup_mesh(t);
        mesh.transform.scale /= p_size;
        draw(mesh, scene, true);

           
    };

};


// Manages asteroids
struct Belt {

    // Caractéristiques de la trajectoire
    float period; // = sqrt(4*pow(3.14,2)*pow(radius_orbit, 3)/(G*mass_parent));
    vcl::vec3 center;  // position du parent
    vcl::vec3 axis; // normalisé à 1 (utiliser normalize())
    float radius_orbit;
    float depth;

    vcl::vec3 diameter_ini; // arbitraire, aucune importance
    // penser à prendre la composante orthonormale à axis

    vcl::vec3 courbure(float angle) {
        if (norm(diameter_ini) == 0 || norm(axis) == 0)
        {
            vcl::call_error("norme non nulle", "définir un vecteur non vide", "Orbit_Object", "position", 22);
        }
        vcl::vec3 axis1 = diameter_ini / norm(diameter_ini);
        vcl::vec3 axis3 = axis / norm(axis);
        vcl::vec3 axis2 = cross(axis3, axis1);
        return (center + std::cos(angle) * radius_orbit * axis1 + std::sin(angle) * radius_orbit * axis2);
    }

    float speed_rotation;

    // Paramètres physiques pour la modélisation de la ceinture d'astéroïdes
    float sigma1; //pour revenir vers l'anneau de loin
    float sigma2; //pour revenir vers l'anneau de près
    float lambda; //pour homomgénéiser les vitesses
    float ka, D; // pour éviter les chocs

    std::vector<Asteroid_Drawable*> elements;

    void update_coord(float dt) {
        for (unsigned int i=0; i < elements.size(); i++) {
            auto& ei = *elements[i];
            for (unsigned int j = i + 1; j < elements.size(); j++) {

                auto &ej = *elements[j];
                vcl::vec3 diff = ei.pos - ej.pos;
                if (vcl::norm(diff) < D) {
                    ei.speed += diff * dt * ka / (ei.mass * pow(vcl::norm(diff), 2));
                    ej.speed += -diff * dt * ka / (ej.mass * pow(vcl::norm(diff), 2));
                }
            }
            // Calculate the projected position on the normal orbit
            vcl::vec3 projax = ei.pos - axis * vcl::dot(ei.pos, axis);
            float dst = vcl::norm(projax);
            projax = projax / dst * radius_orbit; 

            ei.speed += -(ei.speed - speed_rotation * vcl::normalize(vcl::cross(axis, ei.pos))) * dt * lambda / ei.mass;   //utheta au projeté vaut normalize(cross(axis, position))

            // A force brings asteroids back to their orbit
            float ellip = 800.0f;
            if (ellip * std::pow(vcl::dot(ei.pos - projax, axis), 2) +  (radius_orbit - dst)*(radius_orbit - dst) > depth*depth) ei.pos += ei.speed * dt - (ei.pos - projax) * dt * sigma1 / ei.mass;
            else ei.pos += ei.speed * dt - (ei.pos - projax) * dt * sigma2 / ei.mass;

        }
    }
};

void init_orbit_circ(Orbit_Object& obj, float parent_mass, vcl::vec3 initial_position, vcl::vec3 ax) {

    if (parent_mass == 0) throw std::invalid_argument("Parent cannot be massless when initializing orbit.");
    if (is_equal(initial_position, { 0.0f, 0.0f, 0.0f })) throw std::invalid_argument("Orbits of radius 0 are not permitted.");





    if (is_equal(ax, { 0.0f, 0.0f, 0.0f })) {
        if (initial_position.x == 0) {
            ax = { 1.0f, 0.0f, 0.0f };
        }
        else {
            ax = { initial_position.y, -initial_position.x, 0 };
        }
    }

    obj.axis = ax / vcl::norm(ax);
    obj.radius_orbit = vcl::norm(initial_position);
    obj.diameter_ini = initial_position / obj.radius_orbit;

    obj.period = sqrt(4 * pow(3.14, 2) * pow(obj.radius_orbit, 3) / (G * (parent_mass)));

}



#endif // ORBIT_OBJECT_H
