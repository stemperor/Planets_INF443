#include "orbit_object.h"
#include <random>

Asteroid create_ast(float M, vcl::vec3 position_ini, vcl::vec3 speed_ini, float radius){
    Asteroid ast ;

    ast.mass = M;
    ast.position = position_ini;
    ast.speed = speed_ini ;

    // mettre à jour vcl::mesh apparence tq boule + bruit de perlin
    int N = 50;
    ast.apparence = vcl::mesh_primitive_sphere(radius, ast.position, N, N);

    return ast;
}

vcl::vec3 generate_rand_position(float R, float depth, vcl::vec3 Ex, vcl::vec3 Ez){
    vcl::vec3 p;
    std::default_random_engine generator;
    std::normal_distribution<float> distribution1(R,depth);
    float rad = distribution1(generator);
    float phi = vcl::rand_interval(0, 2*3.14);
    std::normal_distribution<float> distribution2(0,depth/R);
    float theta = distribution2(generator);
    p = rad*std::sin(theta)*std::cos(phi)*Ex + rad*std::sin(theta)*std::sin(phi)*vcl::cross(Ez, Ex) + rad*std::cos(theta)*Ez;
    return p;
}

vcl::vec3 generate_rand_speed(vcl::vec3 speed_ini, float rand_speed){
    vcl::vec3 speed ;
    std::default_random_engine generator;
    std::normal_distribution<float> distributionx(speed_ini.x,rand_speed);
    std::normal_distribution<float> distributiony(speed_ini.y,rand_speed);
    std::normal_distribution<float> distributionz(speed_ini.z,rand_speed);
    speed.x = distributionx(generator);
    speed.y = distributiony(generator);
    speed.z = distributionz(generator);
    return speed;
}

Belt create_belt(Planete_Drawable& parent, vcl::vec3 ax, float R, float depth, int N, float mass_ast, float radius_ast, float rand_speed){
    // N number of asteroids

    ///
    /// //////////////
    /// 
    double parentmass = 10;

    Belt ceinture;

    ceinture.center = vcl::vec3();
    ceinture.radius_orbit = R;
    ceinture.axis = vcl::normalize(ax);
    ceinture.period = sqrt(4*pow(3.14,2)*pow(ceinture.radius_orbit, 3)/(G*parentmass));
    if (vcl::is_equal(ceinture.axis, {1.0f, 0.0f, 0.0f}))
        ceinture.diameter_ini = vcl::normalize(vcl::cross(ceinture.axis, {0.0f,1.0f,0.0f}));
    else ceinture.diameter_ini = vcl::normalize(vcl::cross(ceinture.axis, {1.0f,0.0f,0.0f}));

    // Paramètres à optimiser ici
    ceinture.sigma = 1.0f;
    ceinture.lambda = 1.0f;
    ceinture.ka = 1.0f;

    // Paramètre à optimiser en fonction du rayon des astéroïdes
    ceinture.D = 4*radius_ast ;


    for (unsigned int i ; i<N ; i++){
        vcl::vec3 p;
        bool b = true ;
        while (b) {
            p = generate_rand_position(R, depth, ceinture.diameter_ini, ceinture.axis);
            b = false ;
            for (unsigned int j ; j< ceinture.elements.size(); j++){
                if (vcl::norm(ceinture.elements[j].position - p)<3*radius_ast)
                    b=true ;
            }
        }
        vcl::vec3 v = generate_rand_speed(ceinture.speed_rotation*vcl::normalize(vcl::cross(ceinture.axis, p)), rand_speed);
        float M = vcl::rand_interval(mass_ast*3/4, mass_ast*5/4);
        Asteroid ast = create_ast(M, p, v, radius_ast);
        ast.angle_projection = std::acos(vcl::dot(ast.position, ceinture.diameter_ini)/vcl::norm(ast.position - vcl::dot(ast.position, ceinture.axis)*ceinture.axis)) ;
        ceinture.elements.push_back(ast);
    }
    return ceinture;
}
