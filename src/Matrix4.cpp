#include "Matrix4.h"
#include <cmath>
#include <iomanip>

// Default constructor initializes to 4x4 Identity matrix
Matrix4::Matrix4() {
    for (int i = 0; i < 16; ++i) {
        elements[i] = 0.0f;
    }
    // Set diagonal elements to 1.0f
    elements[0]  = 1.0f; // Col 0, Row 0
    elements[5]  = 1.0f; // Col 1, Row 1
    elements[10] = 1.0f; // Col 2, Row 2
    elements[15] = 1.0f; // Col 3, Row 3
}

// Return 4x4 Identity matrix
Matrix4 Matrix4::identity() {
    return Matrix4();
}

// Element access: Column-major storage indexing
// Index in array = (col * 4) + row
float& Matrix4::at(int row, int col) {
    return elements[col * 4 + row];
}

float Matrix4::at(int row, int col) const {
    return elements[col * 4 + row];
}

/**
 * Rx =
 * [ 1    0      0    0 ]
 * [ 0   cosθ  -sinθ  0 ]
 * [ 0   sinθ   cosθ  0 ]
 * [ 0    0      0    1 ]
 */
Matrix4 Matrix4::rotationX(float angleInRadians) {
    Matrix4 result = Matrix4::identity();
    float c = std::cos(angleInRadians);
    float s = std::sin(angleInRadians);

    result.at(1, 1) =  c;
    result.at(1, 2) = -s;
    result.at(2, 1) =  s;
    result.at(2, 2) =  c;

    return result;
}

/**
 * Ry =
 * [  cosθ   0   sinθ   0 ]
 * [   0     1    0     0 ]
 * [ -sinθ   0   cosθ   0 ]
 * [   0     0    0     1 ]
 */
Matrix4 Matrix4::rotationY(float angleInRadians) {
    Matrix4 result = Matrix4::identity();
    float c = std::cos(angleInRadians);
    float s = std::sin(angleInRadians);

    result.at(0, 0) =  c;
    result.at(0, 2) =  s;
    result.at(2, 0) = -s;
    result.at(2, 2) =  c;

    return result;
}

/**
 * Rz =
 * [ cosθ  -sinθ   0    0 ]
 * [ sinθ   cosθ   0    0 ]
 * [  0      0     1    0 ]
 * [  0      0     0    1 ]
 */
Matrix4 Matrix4::rotationZ(float angleInRadians) {
    Matrix4 result = Matrix4::identity();
    float c = std::cos(angleInRadians);
    float s = std::sin(angleInRadians);

    result.at(0, 0) =  c;
    result.at(0, 1) = -s;
    result.at(1, 0) =  s;
    result.at(1, 1) =  c;

    return result;
}

/**
 * Translation matrix:
 * [ 1  0  0  tx ]
 * [ 0  1  0  ty ]
 * [ 0  0  1  tz ]
 * [ 0  0  0   1 ]
 */
Matrix4 Matrix4::translation(float tx, float ty, float tz) {
    Matrix4 result = Matrix4::identity();
    result.at(0, 3) = tx;
    result.at(1, 3) = ty;
    result.at(2, 3) = tz;
    return result;
}

/**
 * Scaling matrix:
 * [ sx  0   0   0 ]
 * [  0  sy  0   0 ]
 * [  0  0   sz  0 ]
 * [  0  0   0   1 ]
 */
Matrix4 Matrix4::scale(float sx, float sy, float sz) {
    Matrix4 result = Matrix4::identity();
    result.at(0, 0) = sx;
    result.at(1, 1) = sy;
    result.at(2, 2) = sz;
    return result;
}

/**
 * Explicit 4x4 Matrix Multiplication via Summation Formula:
 * C(row, col) = sum_{k=0..3} A(row, k) * B(k, col)
 */
Matrix4 Matrix4::operator*(const Matrix4& other) const {
    Matrix4 result;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += this->at(row, k) * other.at(k, col);
            }
            result.at(row, col) = sum;
        }
    }
    return result;
}

Matrix4 Matrix4::multiply(const Matrix4& other) const {
    return *this * other;
}

/**
 * Transform 4D homogeneous point/vector:
 * v' = M * v
 */
Vector4 Matrix4::transformPoint(float x, float y, float z, float w) const {
    Vector4 result;
    result.x = at(0, 0)*x + at(0, 1)*y + at(0, 2)*z + at(0, 3)*w;
    result.y = at(1, 0)*x + at(1, 1)*y + at(1, 2)*z + at(1, 3)*w;
    result.z = at(2, 0)*x + at(2, 1)*y + at(2, 2)*z + at(2, 3)*w;
    result.w = at(3, 0)*x + at(3, 1)*y + at(3, 2)*z + at(3, 3)*w;
    return result;
}

Vector4 Matrix4::transformPoint(const Vector4& v) const {
    return transformPoint(v.x, v.y, v.z, v.w);
}

const float* Matrix4::data() const {
    return elements;
}

float* Matrix4::data() {
    return elements;
}

void Matrix4::print(const std::string& label) const {
    if (!label.empty()) {
        std::cout << label << ":\n";
    }
    for (int row = 0; row < 4; ++row) {
        std::cout << "[ ";
        for (int col = 0; col < 4; ++col) {
            std::cout << std::setw(9) << std::fixed << std::setprecision(4) << at(row, col) << " ";
        }
        std::cout << "]\n";
    }
}
