#pragma once

#include <set>
#include <vector>
#include "Geometry.h"
#include "GeometricTransformations.h"
#include "DrawingSupport.h"

class ShadowVolume
{
public:
	ShadowVolume();
	ShadowVolume(size_t w, size_t h, const LightSource& ls);
	ShadowVolume(const ShadowVolume& other);
	~ShadowVolume();

	ShadowVolume& operator=(const ShadowVolume& other);

	size_t GetHeight() const;
	size_t GetWidth() const;
	void SetSize(size_t w, size_t h);
	void Clear();
	void SetPixel(int x, int y, double z, COLORREF id);
	void SetLightSource(const LightSource& ls);

	bool IsPixelLit(size_t x, size_t y, double z) const;

	void ProcessModel(const PolygonalModel& model, const MatrixHomogeneous& mTotal, const std::vector<Normals::PolygonNormalData>& normals, bool clip, const ClippingPlane& cp, const PolygonAdjacencyGraph& polygonAdj);
	std::pair<PolygonalObject, std::vector<Normals::PolygonNormalData>> GenerateShadowVolume(const PolygonalModel& model, const std::vector<Normals::PolygonNormalData>& normals, const PolygonAdjacencyGraph& polygonAdj) const;

private:
	enum ShadowEventType { ShadowEnter, ShadowExit };

	class ShadowEvent
	{
	public:
		ShadowEvent(ShadowEventType t, double z, COLORREF id);
		~ShadowEvent();

		ShadowEventType _type;
		double _z;
		COLORREF _id;
		bool operator < (const ShadowEvent& other) const;
	};

private:
	size_t _height, _width;
	std::set<ShadowEvent>* _stencil;
	LightSource _lightSource;
	ShadowEventType _currShadowMode;
	int _currPolyId = 0;
};

