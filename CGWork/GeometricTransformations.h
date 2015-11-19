#ifndef _GEOMETRIC_TRANSFORMATION_H
#define _GEOMETRIC_TRANSFORMATION_H

#include "Geometry.h"

template <typename T, int D>
class MatrixBase
{
public:
	MatrixBase()
	{	}

	MatrixBase(T rows[D])
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
			for (int j = 0; j < D; ++i)
			{
				cols[i][j] = rows[j][i];
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

	T operator*(const T& other) const
	{
		T res;
		for (int i = 0; i < D; ++i)
		{
			res[i] = _rows[i] * T;
		}
		return res;
	}

	MatrixBase operator*(const MatrixBase& other) const
	{
		T resRows[D];
		MatrixBase otherT = other.Transposed();
		for (int i = 0; i < D; ++i)
		{
			for (int j = 0; j < D; ++i)
			{
				resRows[i][j] = _rows[i] * otherT._rows[i];
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

#endif
