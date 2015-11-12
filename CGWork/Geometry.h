#pragma once

class Point3D
{
public:
	Point3D();
	Point3D(double x_, double y_, double z_);
	Point3D(const Point3D& other);

public:
	Point3D operator+(const Point3D& other) const;
	Point3D operator-(const Point3D& other) const;
	Point3D operator-() const;
	double operator*(const Point3D& other) const;
	Point3D operator*(double s) const;
	Point3D operator/(double s) const;
	Point3D Cross(const Point3D& other) const;

	double Norm() const;
	double SquareNorm() const;
	Point3D Normalized() const;

public:
	double x, y, z;
};

Point3D operator*(double s, const Point3D& p);