#include "GeometricTransformations.h"

Matrix3D ToMatrix3D(const Vector3D& r0, const Vector3D& r1, const Vector3D& r2)
{
	Vector3D rows[3] = { r0, r1, r2 };
	return Matrix3D(rows);
}

MatrixHomogeneous ToMatrixHomogeneous(const HomogeneousPoint& r0, const HomogeneousPoint& r1, const HomogeneousPoint& r2, const HomogeneousPoint& r3)
{
	HomogeneousPoint rows[4] = { r0, r1, r2, r3 };
	return MatrixHomogeneous(rows);
}

