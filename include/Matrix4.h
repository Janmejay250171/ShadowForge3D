#ifndef SHADOWFORGE_MATRIX4_H
#define SHADOWFORGE_MATRIX4_H

#include <iostream>
#include <string>

/**
 * Homogeneous 4D vector (x, y, z, w).
 * For 3D positions, w = 1.0f.
 * For 3D directions / vectors, w = 0.0f.
 */
struct Vector4 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    Vector4() = default;
    Vector4(float x, float y, float z, float w = 1.0f) : x(x), y(y), z(z), w(w) {}
};

/**
 * Custom 4x4 Transformation Matrix class.
 * 
 * Column-Vector Convention:
 *   v' = M * v
 * 
 * Internal Memory Layout:
 *   Stored as a contiguous 16-element float array in column-major order,
 *   matching standard OpenGL shader uniform layout expectations:
 *     elements[col * 4 + row]
 * 
 * Mathematical Rotation Matrices:
 * 
 *   Rx(θ) =
 *   [ 1      0        0       0 ]
 *   [ 0    cosθ    -sinθ      0 ]
 *   [ 0    sinθ     cosθ      0 ]
 *   [ 0      0        0       1 ]
 * 
 *   Ry(θ) =
 *   [  cosθ   0    sinθ       0 ]
 *   [   0     1     0         0 ]
 *   [ -sinθ   0    cosθ       0 ]
 *   [   0     0     0         1 ]
 * 
 *   Rz(θ) =
 *   [ cosθ  -sinθ   0         0 ]
 *   [ sinθ   cosθ   0         0 ]
 *   [  0      0     1         0 ]
 *   [  0      0     0         1 ]
 */
class Matrix4 {
private:
    // 16 floats stored in column-major order:
    // col 0: [0..3], col 1: [4..7], col 2: [8..11], col 3: [12..15]
    float elements[16];

public:
    // Default constructor: initializes to 4x4 Identity matrix
    Matrix4();

    // Factory methods for standard transformations
    static Matrix4 identity();
    static Matrix4 rotationX(float angleInRadians);
    static Matrix4 rotationY(float angleInRadians);
    static Matrix4 rotationZ(float angleInRadians);
    static Matrix4 translation(float tx, float ty, float tz);
    static Matrix4 scale(float sx, float sy, float sz);

    // Element access by (row, column) using 0-based indices [0..3]
    float& at(int row, int col);
    float at(int row, int col) const;

    // Explicit 4x4 matrix multiplication using the summation formula
    // C(row, col) = sum_{k=0..3} A(row, k) * B(k, col)
    Matrix4 operator*(const Matrix4& other) const;
    Matrix4 multiply(const Matrix4& other) const;

    // Transform a 4D homogeneous point: v' = M * v
    Vector4 transformPoint(float x, float y, float z, float w = 1.0f) const;
    Vector4 transformPoint(const Vector4& v) const;

    // Raw pointer access for passing directly to glUniformMatrix4fv
    const float* data() const;
    float* data();

    // Debug print function for console inspection
    void print(const std::string& label = "") const;
};

#endif // SHADOWFORGE_MATRIX4_H
