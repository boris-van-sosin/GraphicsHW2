#include "Drawing.h"

MatrixHomogeneous ScaleAndCenter(const BoundingBox boundingCube)
{
	const double right = boundingCube.maxX;
	const double left = boundingCube.minX;
	const double bottom = boundingCube.minY;
	const double top = boundingCube.maxY;
	const double far = -boundingCube.maxZ;
	const double near = -boundingCube.minZ;
	HomogeneousPoint rows[4] = {
		HomogeneousPoint(2 / (right - left), 0, 0, -(right + left) / (right - left)),
		HomogeneousPoint(0, 2 / (top - bottom), 0, -(top + bottom) / (top - bottom)),
		HomogeneousPoint(0, 0, 2 / (near - far), -(near + far) / (far - near)),
		HomogeneousPoint(0, 0, 0, 1)
	};
	return MatrixHomogeneous(rows);
}

MatrixHomogeneous PerspectiveWarpMatrix(const BoundingBox boundingCube)
{
	const double far = -boundingCube.maxZ*3;
	const double near = -boundingCube.minZ*3;
	const double alpha = (near + far) / (far - near);
	//const double d = -near * 2;
	//const double a = -far * 2;
	const double beta = 2 * near*far / (near - far);
	HomogeneousPoint rows[] = {
		HomogeneousPoint(1, 0, 0, 0),
		HomogeneousPoint(0, 1, 0, 0),
		HomogeneousPoint(0, 0, alpha, beta),
		HomogeneousPoint(0, 0, -1, 0)
	};

	//return MatrixHomogeneous(rows) * ScaleAndCenter(boundingCube);
	HomogeneousPoint rows2[] = {
		HomogeneousPoint(1, 0, 0, 0),
		HomogeneousPoint(0, 1, 0, 0),
		HomogeneousPoint(0, 0, 1, 0),
		HomogeneousPoint(0, 0, 1, 0)
	};
	return MatrixHomogeneous(rows2) * Matrices::Translate(0, 0, 2) * ScaleAndCenter(boundingCube);
}

MatrixHomogeneous OrthographicProjectMatrix(const BoundingBox boundingCube)
{
	HomogeneousPoint rows[4] = {
		HomogeneousPoint(1, 0, 0, 0),
		HomogeneousPoint(0, 1, 0, 0),
		HomogeneousPoint(0, 0, 0, 0),
		HomogeneousPoint(0, 0, 0, 1)
	};
	return MatrixHomogeneous(rows) * ScaleAndCenter(boundingCube);
}
