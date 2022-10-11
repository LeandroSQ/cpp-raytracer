#pragma once
#include "math.hpp"

class Color {
  public:
	float red;
	float green;
	float blue;

    Color(float red, float green, float blue): red(clamp(red, 0.0f, 1.0f)), green(clamp(green, 0.0f, 1.0f)), blue(clamp(blue, 0.0f, 1.0f)) { }

    Color operator*(float scalar) {
        return Color(red * scalar, green * scalar, blue * scalar);
    }

    Color operator+(Color other) {
        return Color(red + other.red, green + other.green, blue + other.blue);
    }

    Color operator-(Color other) {
        return Color(red - other.red, green - other.green, blue - other.blue);
    }

    static Color mix(Color a, Color b, float mixture) {
        mixture = clamp(mixture, 0.0f, 1.0f);

        return Color(
            a.red * (1.0f - mixture) + b.red * mixture,
            a.green * (1.0f - mixture) + b.green * mixture,
            a.blue * (1.0f - mixture) + b.blue * mixture
        );
    }

};