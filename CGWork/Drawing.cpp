#include "Drawing.h"

MatrixHomogeneous ScaleAndCenter(const BoundingBox& boundingCube)
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

MatrixHomogeneous PerspectiveWarpMatrix(const BoundingBox& boundingCube)
{

	const double right = boundingCube.maxX * 4;
	const double left = boundingCube.minX * 4;
	const double bottom = boundingCube.minY * 4;
	const double top = boundingCube.maxY * 4;
	const double depth = boundingCube.maxZ - boundingCube.minZ;
	const double near = depth;
	const double far = near + 2 * (depth);

	const double fH = top - bottom;
	const double fW = right - left;

	const double h = 2 * near / fH;
	const double w = 2 * near / fW;
	const double q = far / (far - near);

	HomogeneousPoint rows[] = {
		HomogeneousPoint(w, 0, 0, 0),
		HomogeneousPoint(0, h, 0, 0),
		HomogeneousPoint(0, 0, q, 1),
		HomogeneousPoint(0, 0, -q*near, 0)
	};

	const double clippingMargin = depth * 0.1;

	const double near2 = 1 - clippingMargin;
	const double far2 = near + depth + clippingMargin;
	const double q2 = far2 / (far2 - near2);

	//return MatrixHomogeneous(rows) * Matrices::Translate(0, 0, boundingCube.minZ + near + depth*0.5);
	HomogeneousPoint rows2[] = {
		HomogeneousPoint(3, 0, 0, 0),
		HomogeneousPoint(0, 3, 0, 0),
		HomogeneousPoint(0, 0, q, 1),
		HomogeneousPoint(0, 0, -q*near2, 0)
	};

	return MatrixHomogeneous(rows2) * Matrices::Translate(0, 0, 2) * ScaleAndCenter(boundingCube);
}

MatrixHomogeneous OrthographicProjectMatrix(const BoundingBox& boundingCube)
{
	HomogeneousPoint rows[4] = {
		HomogeneousPoint(1, 0, 0, 0),
		HomogeneousPoint(0, 1, 0, 0),
		HomogeneousPoint(0, 0, 0, 0),
		HomogeneousPoint(0, 0, 0, 1)
	};
	return MatrixHomogeneous(rows) * ScaleAndCenter(boundingCube);
}
