#ifndef _GEOMETRIC_TRANSFORMATION_H
#define _GEOMETRIC_TRANSFORMATION_H

#include "Geometry.h"

template <typename T, int D>
class MatrixBase
{
public:
	MatrixBase()
	{	}

	MatrixBase(const T rows[D])
	{
		for (int i = 0; i < D; ++i)
		{
			_rows[i] = rows[i];
		}
	}

	MatrixBase(const MatrixBase& other)
		: MatrixBase(other._rows)
	{
	}

	MatrixBase Transposed() const
	{
		T cols[D];
		for (int i = 0; i < D; ++i)
		{
			for (int j = 0; j < D; ++j)
			{
				cols[i][j] = _rows[j][i];
			}
		}
		return MatrixBase(cols);
	}

	MatrixBase operator+(const MatrixBase& other) const
	{
		T resRows[D];
		for (int i = 0; i < D; ++i)
		{
			resRows[i] = _rows[i] + other._rows[i];
		}
		return MatrixBase(resRows);
	}

	T operator*(const T& vec) const
	{
		T res;
		for (int i = 0; i < D; ++i)
		{
			res[i] = _rows[i] * vec;
		}
		//
		if (D == 4 && res[3] == 0)
		{
			int k = 6;
		}
		//
		return res;
	}

	MatrixBase operator*(const MatrixBase& other) const
	{
		T resRows[D];
		MatrixBase otherT = other.Transposed();
		for (int i = 0; i < D; ++i)
		{
			for (int j = 0; j < D; ++j)
			{
				resRows[i][j] = _rows[i] * otherT._rows[j];
			}
		}
		return MatrixBase(resRows);
	}

	MatrixBase operator*(double x) const
	{
		T resRows[D];
		for (int i = 0; i < D; ++i)
		{
			resRows[i] = _rows[i] * x;
		}
		return MatrixBase(resRows);
	}

	MatrixBase& operator=(const MatrixBase& other)
	{
		for (int i = 0; i < D; ++i)
		{
			_rows[i] = other._rows[i];
		}
		return *this;
	}

private:
	T _rows[D];
};

template <typename T, int D>
MatrixBase<T, D> operator*(double x, const MatrixBase<T, D>& m)
{
	return m*x;
}

typedef MatrixBase<Vector3D, 3> Matrix3D;
typedef MatrixBase<HomogeneousPoint, 4> MatrixHomogeneous;

Matrix3D ToMatrix3D(const Vector3D& r0, const Vector3D& r1, const Vector3D& r2);
MatrixHomogeneous ToMatrixHomogeneous(const HomogeneousPoint& r0, const HomogeneousPoint& r1, const HomogeneousPoint& r2, const HomogeneousPoint& r3);

Polygon3D operator*(const Matrix3D& m, const Polygon3D& poly);
Polygon3D operator*(const MatrixHomogeneous& m, const Polygon3D& poly);
PolygonalObject operator*(const Matrix3D& m, const PolygonalObject& obj);
PolygonalObject operator*(const MatrixHomogeneous& m, const PolygonalObject& obj);

const Matrix3D ZerosMatrix3D;
const Matrix3D UnitMatrix3D = ToMatrix3D(Vector3D(1, 0, 0), Vector3D(0, 1, 0), Vector3D(0, 0, 1));

enum Axis { AXIS_X, AXIS_Y, AXIS_Z };

namespace Matrices
{
	const MatrixHomogeneous ZerosMatrixHomogeneous;
	const MatrixHomogeneous UnitMatrixHomogeneous = ToMatrixHomogeneous(HomogeneousPoint(1, 0, 0, 0), HomogeneousPoint(0, 1, 0, 0), HomogeneousPoint(0, 0, 1, 0), HomogeneousPoint(0, 0, 0, 1));
	const MatrixHomogeneous ZerosWithW1MatrixHomogeneous = ToMatrixHomogeneous(HomogeneousPoint(0, 0, 0, 0), HomogeneousPoint(0, 0, 0, 0), HomogeneousPoint(0, 0, 0, 0), HomogeneousPoint(0, 0, 0, 1));

	MatrixHomogeneous Scale(double x, double y, double z);
	MatrixHomogeneous Scale(double s);
	MatrixHomogeneous Rotate(Axis axis, double angle);
	MatrixHomogeneous Translate(double x, double y, double z);
	MatrixHomogeneous Flip(Axis axis);
}


#endif
