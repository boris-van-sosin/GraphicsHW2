#pragma once

#include <set>
#include <vector>
#include "Geometry.h"
#include "GeometricTransformations.h"
#include "DrawingSupport.h"

class ShadowVolume
{
public:
	ShadowVolume(size_t w, size_t h, const LightSource& ls);
	~ShadowVolume();

	enum ShadowEventType { ShadowEnter, ShadowExit};

	class ShadowEvent
	{
	public:
		ShadowEvent(ShadowEventType t, double z);
		~ShadowEvent();

		ShadowEventType _type;
		double _z;
		bool operator < (const ShadowEvent& other) const;
	};

	size_t GetHeight() const;
	size_t GetWidth() const;
	void SetSize(size_t w, size_t h);
	void Clear();
	void SetPixel(int x, int y, double z);

	bool IsPixelLit(size_t x, size_t y, double z) const;

	void ProcessModel(const PolygonalModel& model, const MatrixHomogeneous& mTotal, const ModelAttr& attr, const std::vector<Normals::PolygonNormalData>& normals, size_t normalsOffset, bool clip, const ClippingPlane& cp, const PolygonAdjacencyGraph& polygonAdj);

private:
	PolygonalObject GenerateShadowVolume(const PolygonalModel& model, const MatrixHomogeneous& m, const std::vector<Normals::PolygonNormalData>& normals, const PolygonAdjacencyGraph& polygonAdj) const;

private:
	size_t _height, _width;
	std::set<ShadowEvent>* _stencil;
	LightSource _lightSource;
	ShadowEventType _currShadowMode;
};

