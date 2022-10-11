#pragma once
#include "camera.hpp"
#include "sphere.hpp"
#include "vector.hpp"
#include <vector>

class World {
  public:
	std::vector<Sphere> spheres;
	Camera camera;
	Vector3 light;

    World() {
        // Setup camera
        camera.origin.z = 2.0f; // Focal length

        // Setup light
        light = Vector3::normalize(Vector3(-1, -1, -1));

        // Setup spheres
        spheres.push_back(Sphere(Vector3(0, 0, -1), 0.5f, Color(1.0f, 0.0f, 0.0f)));
        spheres.push_back(Sphere(Vector3(-1, 1, -1), 0.25f, Color(0.0f, 1.0f, 0.0f)));
        spheres.push_back(Sphere(Vector3(1, -1, -1), 0.25f, Color(0.0f, 0.0f, 1.0f)));
    }

};