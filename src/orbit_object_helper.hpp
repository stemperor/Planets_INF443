
#include <random>
#include <chrono>

#include "scene_initializer.hpp"


std::default_random_engine generator;
static int num_ast_mesh = 0;

struct perlin_noise_parameters
{
    float persistency = 1.1f;
    float frequency_gain = 2.3f;
    int octave = 2;
    float terrain_height = 1.5f;
};

perlin_noise_parameters parameters;





vcl::mesh_drawable& create_ast_mesh(float radius) {


    vcl::mesh sphere = vcl::mesh_primitive_sphere(radius);

    int N = sphere.position.size();

    for (int k = 0; k < N; k++) {
        vcl::vec3 n0 = vcl::normalize(sphere.position[k]);
        vcl::vec3 n = (n0 + vcl::vec3(1, 1, 1)) / 2;

        float const noiseX = noise_perlin({ pow(n.y,2), pow(n.z,2) }, parameters.octave, parameters.persistency, parameters.frequency_gain);
        float const noiseY = noise_perlin({ pow(n.x,2), pow(n.z,2) }, parameters.octave, parameters.persistency, parameters.frequency_gain);
        float const noiseZ = noise_perlin({ pow(n.x,2), pow(n.y,2) }, parameters.octave, parameters.persistency, parameters.frequency_gain);

        float const noise = (noiseX + noiseY + noiseZ) / 3;

        sphere.position[k] = parameters.terrain_height * noise * radius * n0;
        sphere.color[k] = 0.3f * vec3(0.8f, 0.8f, 0.7f) + 0.1f * noise * vec3(1, 1, 1);
    }
    sphere.compute_normal();

    vcl::mesh_drawable* m = new vcl::mesh_drawable(sphere);

    Scene_initializer s = Scene_initializer::getInstance();

    s.add_mesh(m, "asteroid_"+ std::to_string(num_ast_mesh));

    num_ast_mesh += 1;

    return *m;
}


Asteroid_Drawable * create_ast(float M, vcl::vec3 position_ini, vcl::vec3 speed_ini, float radius) {

    Scene_initializer s = Scene_initializer::getInstance();
    Asteroid_Drawable* ast = new Asteroid_Drawable(create_ast_mesh(1.0f));
    


    ast->mass = M;
    ast->pos = position_ini;
    ast->speed = speed_ini;
    ast->radius = radius;

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

Belt create_belt(Object_Drawable* parent, float parentmass, vcl::vec3 ax, float R, float depth, int N, float mass_ast, float radius_ast, float rand_speed) {
    // N number of asteroids

    ///
    /// //////////////
    /// 

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
    ceinture.D = 1.7 * parameters.terrain_height * radius_ast;

    ceinture.speed_rotation = 2 * 3.14 * ceinture.radius_orbit / ceinture.period;

    print(ceinture.speed_rotation, "speed rot");
    print(ceinture.radius_orbit, "rad");
    print(ceinture.period, "perdiod");
    print(G, "G");

    for (unsigned int i = 0; i < N; i++) {
        vcl::vec3 p;
        bool b = true;
        while (b) {
            p = generate_rand_position(R, depth, ceinture.diameter_ini, ceinture.axis);
            std::cout << p << std::endl;
            b = false;
            for (unsigned int j = 0; j < ceinture.elements.size(); j++) {
                if (vcl::norm(ceinture.elements[j]->pos - p) < 3 * radius_ast)
                    b = true;
            }
        }
        vcl::vec3 v = generate_rand_speed(ceinture.speed_rotation * vcl::normalize(vcl::cross(ceinture.axis, p)), rand_speed);
        float M = vcl::rand_interval(mass_ast * 3 / 4, mass_ast * 5 / 4);
        Asteroid_Drawable * ast = create_ast(M, p, v, radius_ast);
        ast->name = "ast_" + std::to_string(ceinture.elements.size());
        parent->enfants.push_back(ast);
        ast->parent = parent;

        ceinture.elements.push_back(ast);
    }
    return ceinture;
}
