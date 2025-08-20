#include <cmath>
#include <tuple>
#include <algorithm>
#include "pngHelper.h"


// Define constants for the D65 white point (standard illuminant).
const double Xn = 95.047;
const double Yn = 100.0;
const double Zn = 108.883;

// Helper function for the f(t) used in the Lab conversion.
double f(double t) {
    const double delta = 6.0 / 29.0;
    if (t > std::pow(delta, 3)) {
        return std::cbrt(t); // Cube root
    } else {
        return (t / (3 * delta * delta)) + (4.0 / 29.0);
    }
}

double f_inv(double t) {
    const double delta = 6.0 / 29.0;
    if (t > delta) {
        return t * t * t;
    } else {
        return 3 * delta * delta * (t - 4.0 / 29.0);
    }
}

// Function to convert RGB (0-255) to Lab.
std::tuple<double, double, double> RGBtoLab(int R, int G, int B) {
    // Step 1: Convert RGB (0-255) to [0,1]
    double r = R / 255.0;
    double g = G / 255.0;
    double b = B / 255.0;

    // Step 2: Apply the inverse gamma correction for sRGB
    r = (r > 0.04045) ? std::pow((r + 0.055) / 1.055, 2.4) : r / 12.92;
    g = (g > 0.04045) ? std::pow((g + 0.055) / 1.055, 2.4) : g / 12.92;
    b = (b > 0.04045) ? std::pow((b + 0.055) / 1.055, 2.4) : b / 12.92;

    // Step 3: Convert to XYZ space using the sRGB matrix
    double X = (r * 0.4124564 + g * 0.3575761 + b * 0.1804375) * 100.0;
    double Y = (r * 0.2126729 + g * 0.7151522 + b * 0.0721750) * 100.0;
    double Z = (r * 0.0193339 + g * 0.1191920 + b * 0.9503041) * 100.0;

    // Step 4: Normalize for D65 white point and apply the Lab transformation
    double L_ = 116 * f(Y / Yn) - 16;
    double a_ = 500 * (f(X / Xn) - f(Y / Yn));
    double b_ = 200 * (f(Y / Yn) - f(Z / Zn));

    return std::make_tuple(L_, a_, b_);
}

double clamp256(double d) {
	if (d < 0) {
		d = 0;
	}
	else if (d > 255) {
		d = 255;
	}
	return d;
}

std::tuple<int, int, int> LabToRGB(double L_, double a_, double b_) {
    // Step 1: Convert Lab to XYZ
    double Y = f_inv((L_ + 16) / 116.0);
    double X = f_inv((L_ + 16) / 116.0 + a_ / 500.0);
    double Z = f_inv((L_ + 16) / 116.0 - b_ / 200.0);

    // Scale by the reference white point
    X *= Xn;
    Y *= Yn;
    Z *= Zn;

    // Step 2: Convert XYZ to linear RGB
    double r = X * 0.032406 + Y * -0.015372 + Z * -0.004986;
    double g = X * -0.009689 + Y * 0.018758 + Z * 0.000415;
    double b = X * 0.000557 + Y * -0.002040 + Z * 0.010570;

    // Step 3: Apply gamma correction to convert linear RGB to sRGB
    auto gamma_correct = [](double c) {
        if (c <= 0.0031308) {
            return 12.92 * c;
        } else {
            return 1.055 * std::pow(c, 1 / 2.4) - 0.055;
        }
    };

    r = gamma_correct(r);
    g = gamma_correct(g);
    b = gamma_correct(b);

    // Step 4: Convert the values to the [0, 255] range and clamp
    int R = clamp256(static_cast<int>(r * 255.0));
    int G = clamp256(static_cast<int>(g * 255.0));
    int B = clamp256(static_cast<int>(b * 255.0));

    return std::make_tuple(R, G, B);
}
