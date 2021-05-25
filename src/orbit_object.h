#ifndef ORBIT_OBJECT_H
#define ORBIT_OBJECT_H
#include "vcl/vcl.hpp"

float G = 6.6 ;

struct Orbit_Object {

    // Caractéristiques de l'objet
    float mass ;

    // Caractéristiques de la trajectoire
    float period ; // = sqrt(4*pow(3.14,2)*pow(radius_orbit, 3)/(G*mass_parent));
    vcl::vec3 center ;  // position du parent
    vcl::vec3 axis ; // normalisé à 1 (utiliser normalize())
    float radius_orbit ;

    vcl::vec3 diameter_ini ; // rayon initial, à entrer par l'utilisateur, normalisé à 1 (utiliser normalize())

    vcl::vec3 position(float t){        // quel type pour le temps ?
        if (norm(diameter_ini) == 0 || norm(axis) == 0)
            {vcl::call_error("norme non nulle", "définir un vecteur non vide", "Orbit_Object", "position", 22);}
        float angle = 2*3.14*t/period ;
        vcl::vec3 axis1 = diameter_ini / norm(diameter_ini);
        vcl::vec3 axis3 = axis / norm(axis);
        vcl::vec3 axis2 = cross(axis3, axis1);
        return (center + std::cos(angle)*radius_orbit*axis1 + std::sin(angle)*radius_orbit*axis2);
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

    void init_orbit_circ(Orbit_Object& obj, Orbit_Object& parent, float t, vcl::vec3 ax, float rad, vcl::vec3 diam_ini){
        obj.period = sqrt(4*pow(3.14,2)*pow(obj.radius_orbit, 3)/(G*parent.mass));
        obj.center = parent.position(t);

        // remplir les arguments par des valeurs par défaut si on ne veut pas les modifier
        if (!is_equal(ax, {0.0f, 0.0f, 0.0f})) {obj.axis = ax ;}
        if (!is_equal(diam_ini, {0.0f, 0.0f, 0.0f})) {obj.diameter_ini = diam_ini ;}
        if (rad != 0) {obj.radius_orbit = rad ;}
    }

};

struct Planete_Drawable {       //Il faudrait avoir un meshDrawable quelque part
    Orbit_Object& planete ;
    Planete_Drawable& parent ;
    std::vector<Planete_Drawable> enfants ;

    std::string name ;
    float radius ;
    vcl::vec3 rotation_axis ;
    float rotation_speed ;

    // float rotation_angle_ini : l'angle initial de rotation (ça ne me semble pas très pertinent)

    float rotation_angle(float t){
        return (rotation_speed*t);
    }

    // Caractéristiques de la texture


};

#endif // ORBIT_OBJECT_H
