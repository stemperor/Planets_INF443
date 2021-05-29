#ifndef ORBIT_OBJECT_H
#define ORBIT_OBJECT_H
#include "vcl/vcl.hpp"
#include <stdexcept>
#include "draw_helper.hpp"
#include <random>
#include <chrono>


float G = 1;
std::default_random_engine generator;


void print(float s, std::string mess = "") {
    std::cout << mess << ": " << s << std::endl;
};

struct Orbit_Object {

    // Caractéristiques de l'objet
    float mass ;

    // Caractéristiques de la trajectoire
    float period ; // = sqrt(4*pow(3.14,2)*pow(radius_orbit, 3)/(G*mass_parent));
    vcl::vec3 axis ; // normalisé à 1 (utiliser normalize())
    float radius_orbit ;

    vcl::vec3 diameter_ini ; // rayon initial, à entrer par l'utilisateur, normalisé à 1 (utiliser normalize())

    vcl::vec3 position(float t){        // quel type pour le temps ?
        if (norm(diameter_ini) == 0 || norm(axis) == 0)
            {vcl::call_error("norme non nulle", "définir un vecteur non vide", "Orbit_Object", "position", 22);}
        float angle = 2*3.14*t/period ;
        vcl::vec3 axis1 = diameter_ini;
        vcl::vec3 axis3 = axis;
        vcl::vec3 axis2 = cross(axis3, axis1);
        return (std::cos(angle)*radius_orbit*axis1 + std::sin(angle)*radius_orbit*axis2);
    }

    vcl::vec3 speed(float t){
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


void init_orbit_circ(Orbit_Object& obj, float parent_mass, vcl::vec3 initial_position, vcl::vec3 ax = { 0.0f, 0.0f, 0.0f }) {

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

    obj.axis = ax/vcl::norm(ax); 
    obj.radius_orbit = vcl::norm(initial_position);
    obj.diameter_ini = initial_position/obj.radius_orbit;

    obj.period = sqrt(4 * pow(3.14, 2) * pow(obj.radius_orbit, 3) / (G * (parent_mass)));
    
}

struct Planete_Drawable {       //Il faudrait avoir un meshDrawable quelque part
    Orbit_Object* planete = nullptr;
    Planete_Drawable* parent = nullptr;
    std::vector<Planete_Drawable*> enfants ;

    std::string name;
    float radius ;
    vcl::vec3 rotation_axis = vcl::vec3(1,0,0);
    float rotation_speed ;

    float rotation_angle(float t){
        return (rotation_speed*t);
    }

    vcl::vec3 position(double t) {
        if (parent == nullptr)
            return { 0,0,0 };
        else
            return planete->position(t) + parent->position(t);
    }

    vcl::vec3 position(double t, vcl::vec3 parent_pos) {
        if (parent == nullptr)
            return vcl::vec3();
        else
            return planete->position(t) + parent_pos;
    }

    // Caractéristiques de la texture

    GLuint texture;
    GLuint shader;
    shading_parameters_phong shading;


    mesh_drawable & mesh;

    Planete_Drawable(mesh_drawable & d) :mesh(d) {}

    void virtual setup_mesh(double t) {
        mesh.shader = shader;
        mesh.shading = shading;

        mesh.transform.rotate = vcl::rotation(rotation_axis, rotation_angle(t));
        mesh.transform.translate = position(t);

        mesh.transform.scale = radius;
        mesh.texture = texture;
    }

    void virtual draw_obj(double t, scene_environment scene) {
        setup_mesh(t);
        draw(mesh, scene, true);
    };

};

struct Earth_Drawable : public Planete_Drawable{       //Il faudrait avoir un meshDrawable quelque part
    
   
    float cloud_rel_speed;

    GLuint cloud_texture;

    GLuint night_texture;
    GLuint bump_texture;
    GLuint spec_texture;

    GLuint cloud_shader;

    shading_parameters_phong cloud_shading;

    double cloud_height = 0.05;

    Earth_Drawable(mesh_drawable & d) :Planete_Drawable(d) {}

    void virtual draw_obj(double t, scene_environment scene) {
        setup_mesh(t);
        drawearth(mesh, scene, night_texture, spec_texture, bump_texture);

        mesh.transform.scale += cloud_height;
        //mesh.transform.rotate = vcl::rotation(rotation_axis, cloud_rel_speed * t) * mesh.transform.rotate;
        mesh.transform.rotate = vcl::rotation(rotation_axis, 0);
        mesh.shading = cloud_shading;
        mesh.shader = cloud_shader;
        mesh.texture = cloud_texture;
        
        draw(mesh, scene, true);

    }

};


struct Sun_Drawable : public Planete_Drawable {


    Sun_Drawable(mesh_drawable& d) :Planete_Drawable(d) {}

    void virtual draw_obj(double t, scene_environment scene) {

        mesh.shader = shader;

        mesh.transform.rotate = vcl::rotation(rotation_axis, rotation_angle(t));
        mesh.transform.translate = position(t);

        mesh.transform.scale = radius;

        drawsun(mesh, scene, t);
    }
};

struct Asteroid {

    float mass;
    vcl::vec3 position;
    vcl::vec3 speed;
    vcl::vec3 projection; // Paramètre sur la courbe de la ceinture du projeté de l'astéroïde

    // Caractéristiques de l'apparence
    vcl::mesh_drawable apparence;

};

struct Belt {

    // Caractéristiques de la trajectoire
    float period; // = sqrt(4*pow(3.14,2)*pow(radius_orbit, 3)/(G*mass_parent));
    vcl::vec3 center;  // position du parent
    vcl::vec3 axis; // normalisé à 1 (utiliser normalize())
    float radius_orbit;

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
    float sigma; //pour revenir vers l'anneau
    float lambda; //pour homomgénéiser les vitesses
    float ka, D; // pour éviter les chocs

    std::vector<Asteroid> elements;

    void update_coord(float dt) {
        for (unsigned int i=0; i < elements.size(); i++) {
            for (unsigned int j = i + 1; j < elements.size(); j++) {
                vcl::vec3 diff = elements[i].position - elements[j].position;
                if (vcl::norm(diff) < D) {
                    elements[i].speed += diff * dt * ka / (elements[i].mass * pow(vcl::norm(diff), 2));
                    elements[j].speed += -diff * dt * ka / (elements[j].mass * pow(vcl::norm(diff), 2));
                }
            }

            elements[i].speed += -(elements[i].speed - speed_rotation * vcl::normalize(vcl::cross(axis, elements[i].position))) * dt * lambda / elements[i].mass;   //utheta au projeté vaut normalize(cross(axis, position))
            elements[i].position += elements[i].speed * dt - (elements[i].position - elements[i].projection) * dt * sigma / elements[i].mass;
            vcl::vec3 projax = elements[i].position - axis * vcl::dot(elements[i].position, axis);
            elements[i].projection = projax / vcl::norm(projax)*radius_orbit;
            //print(elements[i].angle_projection, "rpoj");
        }
    }
};

Belt create_belt(Planete_Drawable& parent, vcl::vec3 ax, float R, float depth, int N, float mass_ast, float radius_ast, float rand_speed);


Asteroid create_ast(float M, vcl::vec3 position_ini, vcl::vec3 speed_ini, float radius) {
    Asteroid ast;

    ast.mass = M;
    ast.position = position_ini;
    ast.speed = speed_ini;

    // mettre à jour vcl::mesh apparence tq boule + bruit de perlin
    int N = 50;
    ast.apparence = vcl::mesh_drawable(vcl::mesh_primitive_sphere(radius, { 0,0,0 }, N, N));

    return ast;
}

vcl::vec3 generate_rand_position(float R, float depth, vcl::vec3 Ex, vcl::vec3 Ez) {
    vcl::vec3 p;
    std::normal_distribution<float> distribution1(R, depth);
    float rad = distribution1(generator);
    float phi = vcl::rand_interval(0, 2 * 3.14);
    std::normal_distribution<float> distribution2(0, depth / R);
    float theta = distribution2(generator);

    p = rad * std::sin(theta) * std::cos(phi) * Ex + rad * std::sin(theta) * std::sin(phi) * vcl::cross(Ez, Ex) + rad * std::cos(theta) * Ez;
    return p;
}

vcl::vec3 generate_rand_speed(vcl::vec3 speed_ini, float rand_speed) {


    vcl::vec3 speed;
    std::normal_distribution<float> distributionx(speed_ini.x, rand_speed);
    std::normal_distribution<float> distributiony(speed_ini.y, rand_speed);
    std::normal_distribution<float> distributionz(speed_ini.z, rand_speed);
    speed.x = distributionx(generator);
    speed.y = distributiony(generator);
    speed.z = distributionz(generator);
    return speed;
}

Belt create_belt(Planete_Drawable& parent, vcl::vec3 ax, float R, float depth, int N, float mass_ast, float radius_ast, float rand_speed) {
    // N number of asteroids

    ///
    /// //////////////
    /// 
    double parentmass = 10;

    Belt ceinture;

    ceinture.center = { 0,0, 0 };
    ceinture.radius_orbit = R;
    ceinture.axis = vcl::normalize(ax);
    ceinture.period = sqrt(4 * pow(3.14, 2) * pow(ceinture.radius_orbit, 3) / (G * parentmass));
    if (vcl::is_equal(ceinture.axis, { 1.0f, 0.0f, 0.0f }))
        ceinture.diameter_ini = vcl::normalize(vcl::cross(ceinture.axis, { 0.0f,1.0f,0.0f }));
    else ceinture.diameter_ini = vcl::normalize(vcl::cross(ceinture.axis, { 1.0f,0.0f,0.0f }));

    // Paramètres à optimiser ici
    ceinture.sigma = 10.0f;
    ceinture.lambda = 1.0f;
    ceinture.ka = 1.0f;

    // Paramètre à optimiser en fonction du rayon des astéroïdes
    ceinture.D = 2.1 * radius_ast;

    ceinture.speed_rotation = 2 * 3.14 * ceinture.radius_orbit / ceinture.period;

    print(ceinture.speed_rotation, "speed rot");
    print(ceinture.radius_orbit, "rad");
    print(ceinture.period, "perdiod");
    print(G, "G");
    

    for (unsigned int i=0; i < N; i++) {
        vcl::vec3 p;
        bool b = true;
        while (b) {
            p = generate_rand_position(R, depth, ceinture.diameter_ini, ceinture.axis);
            std::cout << p << std::endl;
            b = false;
            for (unsigned int j=0; j < ceinture.elements.size(); j++) {
                if (vcl::norm(ceinture.elements[j].position - p) < 3 * radius_ast)
                    b = true;
            }
        }
        vcl::vec3 v = generate_rand_speed(ceinture.speed_rotation * vcl::normalize(vcl::cross(ceinture.axis, p)), rand_speed);
        float M = vcl::rand_interval(mass_ast * 3 / 4, mass_ast * 5 / 4);
        Asteroid ast = create_ast(M, p, v, radius_ast);
        ast.projection = ast.position;
        ceinture.elements.push_back(ast);
    }
    return ceinture;
}


#endif // ORBIT_OBJECT_H
