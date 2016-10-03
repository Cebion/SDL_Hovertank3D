#include <math.h>
#include "matrix.h"

#include <SDL.h>

void Matrix4x4_SetIdentity(matrix4x4_t * matrix)
{
	float *m = matrix->m;
	memset(m, 0, sizeof(matrix->m));
	m[0] = m[5] = m[10] = m[15] = 1.f;
}

void Matrix4x4_SetTranslate(matrix4x4_t * matrix, float x, float y, float z)
{
	float *m = matrix->m;
	memset(m, 0, sizeof(matrix->m));
	m[0] = m[5] = m[10] = m[15] = 1.f;
	m[12] = x; m[13] = y; m[14] = z;
}

void Matrix4x4_SetScale(matrix4x4_t * matrix, float x, float y, float z)
{
	float *m = matrix->m;
	memset(m, 0, sizeof(matrix->m));
	m[0] = x; m[5] = y; m[10] = z; m[15] = 1.f;
}

void Matrix4x4_SetRotate(matrix4x4_t * matrix, float x, float y, float z, float radians)
{
	float *m = matrix->m;
	float c = cosf(radians);
	float s = sinf(radians);
	float ic = 1.f - c;
	float x2 = x*x;
	float y2 = y*y;
	float z2 = z*z;
	float icx = ic*x;
	float icy = ic*y;
	float xs = x*s;
	float ys = y*s;
	float zs = z*s;
	float cx2 = c*x2;
	float cy2 = c*y2;
	float cz2 = c*z2;
	float icxy = icx*y;
	float icxz = icx*z;
	float icyz = icy*z;

	m[ 0] = x2 + cy2 + cz2;
	m[ 1] = icxy - zs;
	m[ 2] = icxz + ys;
	m[ 3] = 0;

	m[ 4] = icxy + zs;
	m[ 5] = cx2 + y2 + cz2;
	m[ 6] = icyz - xs;
	m[ 7] = 0;

	m[ 8] = icxz - ys;
	m[ 9] = icyz + xs;
	m[10] = cx2 + cy2 + z2;
	m[11] = 0;

	m[12] = 0;
	m[13] = 0;
	m[14] = 0;
	m[15] = 1.f;
}

void Matrix4x4_SetScreen(matrix4x4_t * matrix, float width, float height)
{
	float *m = matrix->m;
	memset(m, 0, sizeof(matrix->m));
	m[0] = 2.f/width;
	m[5] = -2.f/height;
	m[10] = m[13] = m[15] = 1.f;
	m[12] = -1.f;
}

#define PI 3.141592653589793f

void Matrix4x4_SetProject(matrix4x4_t * matrix, float fovUp, float fovDown, float fovLeft, float fovRight, float zNear, float zFar)
{
	float *m = matrix->m;
	memset(m, 0, sizeof(matrix->m));
    float d = zFar - zNear;
    float rd = 1.f / d;

	float tanUp = tanf(fovUp);
	float tanDown = tanf(fovDown);
	float tanLeft = tanf(fovLeft);
	float tanRight = tanf(fovRight);
	float xScale = 2.f / (tanLeft + tanRight);
	float yScale = 2.f / (tanUp + tanDown);

	m[0] = xScale;
	m[5] = yScale;
	m[8] = -0.5f * xScale * (tanLeft - tanRight);
	m[9] = 0.5f * yScale * (tanUp - tanDown);
	m[10] = -(zNear + zFar) * rd;
	m[11] = -1.f;
	m[14] = -2.f * zFar * zNear * rd;
}

void Matrix4x4_Multiply(matrix4x4_t * result, const matrix4x4_t * base, const matrix4x4_t * first)
{
	const float *a = base->m;
	const float *b = first->m;
	float *out = result->m;
	float a00 = a[0], a01 = a[1], a02 = a[2], a03 = a[3],
		a10 = a[4], a11 = a[5], a12 = a[6], a13 = a[7],
		a20 = a[8], a21 = a[9], a22 = a[10], a23 = a[11],
		a30 = a[12], a31 = a[13], a32 = a[14], a33 = a[15];

	float b0  = b[0], b1 = b[1], b2 = b[2], b3 = b[3];  
	out[0] = b0*a00 + b1*a10 + b2*a20 + b3*a30;
	out[1] = b0*a01 + b1*a11 + b2*a21 + b3*a31;
	out[2] = b0*a02 + b1*a12 + b2*a22 + b3*a32;
	out[3] = b0*a03 + b1*a13 + b2*a23 + b3*a33;

	b0 = b[4]; b1 = b[5]; b2 = b[6]; b3 = b[7];
	out[4] = b0*a00 + b1*a10 + b2*a20 + b3*a30;
	out[5] = b0*a01 + b1*a11 + b2*a21 + b3*a31;
	out[6] = b0*a02 + b1*a12 + b2*a22 + b3*a32;
	out[7] = b0*a03 + b1*a13 + b2*a23 + b3*a33;

	b0 = b[8]; b1 = b[9]; b2 = b[10]; b3 = b[11];
	out[8] = b0*a00 + b1*a10 + b2*a20 + b3*a30;
	out[9] = b0*a01 + b1*a11 + b2*a21 + b3*a31;
	out[10] = b0*a02 + b1*a12 + b2*a22 + b3*a32;
	out[11] = b0*a03 + b1*a13 + b2*a23 + b3*a33;

	b0 = b[12]; b1 = b[13]; b2 = b[14]; b3 = b[15];
	out[12] = b0*a00 + b1*a10 + b2*a20 + b3*a30;
	out[13] = b0*a01 + b1*a11 + b2*a21 + b3*a31;
	out[14] = b0*a02 + b1*a12 + b2*a22 + b3*a32;
	out[15] = b0*a03 + b1*a13 + b2*a23 + b3*a33;
}


void Matrix4x4_FrontTranslate(matrix4x4_t * matrix, float x, float y, float z)
{
	// TODO optimize
	matrix4x4_t temp;
	Matrix4x4_SetTranslate(&temp, x, y, z);
	Matrix4x4_Multiply(matrix, matrix, &temp);
}

void Matrix4x4_FrontScale(matrix4x4_t * matrix, float x, float y, float z)
{
	// TODO optimize
	matrix4x4_t temp;
	Matrix4x4_SetScale(&temp, x, y, z);
	Matrix4x4_Multiply(matrix, matrix, &temp);
}

void Matrix4x4_FrontRotate(matrix4x4_t * matrix, float x, float y, float z, float radians)
{
	// TODO optimize
	matrix4x4_t temp;
	Matrix4x4_SetRotate(&temp, x, y, z, radians);
	Matrix4x4_Multiply(matrix, matrix, &temp);
}

void Matrix4x4_BackTranslate(matrix4x4_t * matrix, float x, float y, float z)
{
	// TODO optimize
	matrix4x4_t temp;
	Matrix4x4_SetTranslate(&temp, x, y, z);
	Matrix4x4_Multiply(matrix, &temp, matrix);
}

void Matrix4x4_BackScale(matrix4x4_t * matrix, float x, float y, float z)
{
	// TODO optimize
	matrix4x4_t temp;
	Matrix4x4_SetScale(&temp, x, y, z);
	Matrix4x4_Multiply(matrix, &temp, matrix);
}

void Matrix4x4_BackRotate(matrix4x4_t * matrix, float x, float y, float z, float radians)
{
	// TODO optimize
	matrix4x4_t temp;
	Matrix4x4_SetRotate(&temp, x, y, z, radians);
	Matrix4x4_Multiply(matrix, &temp, matrix);
}

