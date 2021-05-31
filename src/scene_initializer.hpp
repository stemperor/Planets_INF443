#ifndef SCENE_INIT_H
#define SCENE_INIT_H

#include "orbit_object.h"
#include <vector>
#include <map>
#include <string>
#include <fstream>


class Scene_initializer
{
public:
    static Scene_initializer& getInstance()
    {
        static Scene_initializer    instance;

        return instance;
    }

    Object_Drawable* get_object(std::string name) {
        return get_object_rec_(name, parent);
    }

    GLuint get_texture(std::string name) {
        return textures[name];
    }

    GLuint get_shader(std::string name) {
        return shaders[name];
    }

    vcl::mesh_drawable & get_mesh(std::string name) {
        return *meshes[name];
    }

    void add_mesh(vcl::mesh_drawable* m, std::string name) {
        meshes[name] = m;
    }

    Object_Drawable* closest_object(vcl::vec3 pos, double t) {
        return closest_object_rec(pos, parent, t);
    }

    Object_Drawable* closest_angle(vcl::vec3 ray, vcl::vec3 c_pos, double t, float angle_select) {

        float dst;
        return closest_angle_rec(ray, c_pos, parent, t, angle_select, dst);
    }

    void draw(double t, scene_environment scene) {
        draw_rec_(t, scene, parent);
    }

    void kill_initializer() {
        delete_rec(parent);
    }

private:
    Scene_initializer() {

        std::ifstream t(".\\src\\shaders\\skyboxshader.frag.glsl");
        std::stringstream buffer;
        buffer << t.rdbuf();


        std::ifstream t2(".\\src\\shaders\\test.frag.glsl");
        std::stringstream buffer2;
        buffer2 << t2.rdbuf();

        std::ifstream t3(".\\src\\shaders\\sunshader.frag.glsl");
        std::stringstream buffer3;
        buffer3 << t3.rdbuf();

        std::ifstream t4(".\\src\\shaders\\sunbillboard.frag.glsl");
        std::stringstream buffer4;
        buffer4 << t4.rdbuf();

        std::ifstream t5(".\\src\\shaders\\sunbillboard.vert.glsl");
        std::stringstream buffer5;
        buffer5 << t5.rdbuf();

        std::ifstream t6(".\\src\\shaders\\sunshine.frag.glsl");
        std::stringstream buffer6;
        buffer6 << t6.rdbuf();

        std::ifstream t7(".\\src\\shaders\\pointer.frag.glsl");
        std::stringstream buffer7;
        buffer7 << t7.rdbuf();


        shaders["Mesh Shader"] = vcl::opengl_create_shader_program(vcl::opengl_shader_preset("mesh_vertex"), vcl::opengl_shader_preset("mesh_fragment"));
        shaders["Earth Shader"] = vcl::opengl_create_shader_program(vcl::opengl_shader_preset("mesh_vertex"), buffer2.str());
        shaders["Skybox Shader"] = vcl::opengl_create_shader_program(vcl::opengl_shader_preset("mesh_vertex"), buffer.str());
        shaders["Sun Shader"] = vcl::opengl_create_shader_program(vcl::opengl_shader_preset("mesh_vertex"), buffer3.str());
        shaders["Sun Billboard Shader"] = vcl::opengl_create_shader_program(buffer5.str(), buffer4.str());
        shaders["Sun Shine Shader"] = vcl::opengl_create_shader_program(buffer5.str(), buffer6.str());
        shaders["Simple Querry"] = vcl::opengl_create_shader_program(vcl::opengl_shader_preset("single_color_vertex"), vcl::opengl_shader_preset("single_color_fragment"));
        shaders["Pointer Shader"] = vcl::opengl_create_shader_program(vcl::opengl_shader_preset("mesh_vertex"), buffer7.str());

        vcl::mesh_drawable::default_shader = shaders["Mesh Shader"];
        vcl::mesh_drawable::default_texture = vcl::opengl_texture_to_gpu(vcl::image_raw{ 1,1,vcl::image_color_type::rgba,{255,255,255,255} });

        vcl::image_raw const im_earth_night = vcl::image_load_png(".\\src\\assets\\8k_earth_nightmap2.png");
        vcl::image_raw const im_earth = vcl::image_load_png(".\\src\\assets\\8k_earth_daymap.png");
        vcl::image_raw const im_stars = vcl::image_load_png(".\\src\\assets\\8k_stars_milky_way.png");

        vcl::image_raw const im_moon = vcl::image_load_png(".\\src\\assets\\2k_moon.png");
        vcl::image_raw const im_mars = vcl::image_load_png(".\\src\\assets\\2k_mars.png");


        vcl::image_raw const im_earth_spec = vcl::image_load_png(".\\src\\assets\\2k_earth_specular_map.png");
        vcl::image_raw const im_earth_norm = vcl::image_load_png(".\\src\\assets\\8k_earth_normal_map.png");
        vcl::image_raw const im_earth_clouds = vcl::image_load_png(".\\src\\assets\\2k_earth_clouds2.png");

        vcl::image_raw const im_sun_shine = vcl::image_load_png(".\\src\\assets\\star_glow.png");

        vcl::image_raw const pointer = vcl::image_load_png(".\\src\\assets\\marker.png");

        // Send this image to the GPU, and get its identifier texture_image_id
        textures["Earth Night"] = opengl_texture_to_gpu(im_earth_night, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        textures["Earth Day"] = opengl_texture_to_gpu(im_earth, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        textures["Stars"] = opengl_texture_to_gpu(im_stars, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        textures["Earth Specular"] = opengl_texture_to_gpu(im_earth_spec, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        textures["Earth Norm"] = opengl_texture_to_gpu(im_earth_norm, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        textures["Earth Clouds"] = opengl_texture_to_gpu(im_earth_clouds, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        textures["Moon"] = opengl_texture_to_gpu(im_moon, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        textures["Mars"] = opengl_texture_to_gpu(im_mars, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        textures["Sun Shine"] = opengl_texture_to_gpu(im_sun_shine, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        textures["Pointer"] = opengl_texture_to_gpu(pointer, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);



        float const r = 1.0f; // TO BE CHANGED
        
        meshes["LQ Sphere"] = new vcl::mesh_drawable(vcl::mesh_primitive_sphere(r, { 0, 0, 0 }, 60, 40));
        meshes["HQ Sphere"] = new vcl::mesh_drawable(vcl::mesh_primitive_sphere(r, { 0, 0, 0 }, 100, 100));

        // Initialize Sun

        parent = new Sun_Drawable(get_mesh("LQ Sphere"));
        parent->name = "Sun";
        parent->radius = 20;

        parent->texture = textures["Sun"];
        parent->shader = shaders["Sun Shader"];

        float sun_mass = 10;

        // Initialize Earth

        Earth_Drawable* Earth = new Earth_Drawable(get_mesh("HQ Sphere"), { 70.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, sun_mass);
        parent->enfants.push_back(Earth);

        Earth->parent = parent;

        Earth->texture = textures["Earth Day"];
        Earth->bump_texture = textures["Earth Norm"];
        Earth->cloud_texture = textures["Earth Clouds"];
        Earth->spec_texture = textures["Earth Specular"];
        Earth->night_texture = textures["Earth Night"];

        Earth->radius = 4.0;
        Earth->name = "Earth";
        Earth->rotation_speed = 0.0;
        Earth->shader = shaders["Earth Shader"];
        Earth->planete->mass = 1;
        Earth->cloud_shader = shaders["Mesh Shader"];

        Earth->shading.phong.specular = 0.2f;
        Earth->shading.phong.specular_exponent = 3.0;
        Earth->shading.phong.ambient = 0.0f;
        Earth->shading.phong.diffuse = 0.7f;

        Earth->cloud_shading.phong.specular = 0.0f;
        Earth->cloud_shading.phong.specular_exponent = 3.0;
        Earth->cloud_shading.phong.ambient = 0.0f;
        Earth->cloud_shading.phong.diffuse = 1.0f;

        Planete_Drawable* moon = new Planete_Drawable(get_mesh("LQ Sphere"), { 2.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, Earth->planete->mass);
        Earth->enfants.push_back(moon);

        moon->parent = Earth;
        moon->name = "Moon";
        moon->radius = 0.8;
        moon->rotation_speed = 0.0;
        moon->shader = shaders["Mesh Shader"];
        moon->planete->mass = 0.5;
        moon->texture = textures["Moon"];


        Planete_Drawable* mars = new Planete_Drawable(get_mesh("LQ Sphere"), { 120.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, sun_mass);
        parent->enfants.push_back(mars);

        mars->parent = parent;
        mars->name = "Mars";
        mars->radius = 4.0;
        mars->rotation_speed = 0.0;
        mars->shader = shaders["Mesh Shader"];
        mars->planete->mass = 1;
        mars->texture = textures["Mars"];




    }

    // Complexité carastrophique, à refaire
    Object_Drawable* closest_object_rec(vcl::vec3 pos, Object_Drawable* p, double t) {
        float mindist = vcl::norm(pos - p->position(t));
        auto minp = p;

        for (auto child : p->enfants)
        {
            auto k = closest_object_rec(pos, child, t);
            float tempmindist = std::min(mindist, vcl::norm(k->position(t) - pos));
            if (tempmindist > mindist) {
                mindist = tempmindist;
                minp = k;
            }
        }
        return minp;
    }



    void delete_rec(Object_Drawable* d) {
        for (auto c : d->enfants) {
            delete_rec(c);
        }

        delete d;
    }


    Object_Drawable* closest_angle_rec(vcl::vec3 ray, vcl::vec3 c_pos, Object_Drawable* p, double t, float angle_select, float& dist_return) {
        vcl::vec3 rel_pos = p->position(t) - c_pos;
        double dist = vcl::norm(rel_pos);
        float angle = std::acos(vcl::dot(rel_pos, ray) / dist);
        float angle_min = std::atan(p->radius * 1.3/ dist);

        std::cout << "angle " << p->name << " " << angle << std::endl;
        std::cout << "angle_min " << angle_min << std::endl;

        Object_Drawable* best = nullptr;

        if (angle < std::max(angle_min, angle_select)) {
            best = p;
        }

        for (auto child : p->enfants)
        {
            float dist_ = 0;
            auto k = closest_angle_rec(ray, c_pos, child, t, angle_select, dist_);

            if (k != nullptr) {
                if (best == nullptr || dist_ < dist) {
                    dist = dist_;
                    best = k;
                }
            }
        }

        dist_return = dist;

        return best;
    }

    Object_Drawable* get_object_rec_(std::string name, Object_Drawable* start) {
        if (start->name == name)
            return start;

        for (auto p : start->enfants){
            auto res = get_object_rec_(name, p);
            if (res != nullptr)
                return res;
        }

        return nullptr;
    }

    void draw_rec_(double t, scene_environment scene, Object_Drawable* p) {
        std::cout << p->name << std::endl;
        p->draw_obj(t, scene);

        for (auto child : p->enfants)
            draw_rec_(t, scene, child);
    }

    Object_Drawable* parent;
    std::map<std::string, GLuint> textures;
    std::map<std::string, GLuint> shaders;
    std::map<std::string, vcl::mesh_drawable*> meshes;


    //void operator=(Scene_initializer const&);

public:
    void operator=(Scene_initializer const&) = delete;
};

#endif