#ifndef MATRIX_H
#define MATRIX_H

typedef struct {
	float m[16];
} matrix4x4_t;

void Matrix4x4_SetIdentity(matrix4x4_t * matrix);

void Matrix4x4_SetTranslate(matrix4x4_t * matrix, float x, float y, float z);

void Matrix4x4_SetScale(matrix4x4_t * matrix, float x, float y, float z);

// rotation is clockwise:
// xaxis rotation is +z towards +y
// yaxis rotation is +x towards +z
// zaxis rotation is +y towards +x

void Matrix4x4_SetRotate(matrix4x4_t * matrix, float x, float y, float z, float radians);

void Matrix4x4_SetScreen(matrix4x4_t * matrix, float width, float height);

void Matrix4x4_SetProject(matrix4x4_t * matrix, float fovUp, float fovDown, float fovLeft, float fovRight, float zNear, float zFar);

// first matrix is transformed by the base matrix
// in the pipeline, a vector is transformed by first matrix and then by base matrix
// examples:
//   Matrix4x4_Multiply(mv, view, model);
//   Matrix4x4_Multiply(mvp, projection, mv);

void Matrix4x4_Multiply(matrix4x4_t * result, const matrix4x4_t * base, const matrix4x4_t * first);

void Matrix4x4_FrontTranslate(matrix4x4_t * matrix, float x, float y, float z);
void Matrix4x4_FrontScale(matrix4x4_t * matrix, float x, float y, float z);
void Matrix4x4_FrontRotate(matrix4x4_t * matrix, float x, float y, float z, float radians);

void Matrix4x4_BackTranslate(matrix4x4_t * matrix, float x, float y, float z);
void Matrix4x4_BackScale(matrix4x4_t * matrix, float x, float y, float z);
void Matrix4x4_BackRotate(matrix4x4_t * matrix, float x, float y, float z, float radians);

#endif//MATRIX_H