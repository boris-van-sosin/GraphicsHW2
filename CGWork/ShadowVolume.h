#pragma once

#include <set>
#include <map>
#include <vector>
#include "Geometry.h"
#include "GeometricTransformations.h"
#include "DrawingSupport.h"

class ShadowVolume
{
public:
	ShadowVolume();
	ShadowVolume(size_t w, size_t h, const LightSource& ls);
	ShadowVolume(size_t w, size_t h, const LightSource& ls, const BoundingBox& wbox);
	ShadowVolume(size_t w, size_t h, const LightSource& ls, double shadowLength);
	ShadowVolume(const ShadowVolume& other);
	~ShadowVolume();

	ShadowVolume& operator=(const ShadowVolume& other);

	size_t GetHeight() const;
	size_t GetWidth() const;
	void SetSize(size_t w, size_t h);
	void SetWorldBox(const BoundingBox& wbox);
	void Clear();
	void SetPixel(int x, int y, double z, COLORREF id);
	void SetLightSource(const LightSource& ls);

	bool IsPixelLit(size_t x, size_t y, double z, const Point3D& pt) const;

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

	class ShadowLimits
	{
	public:
		ShadowLimits();
		ShadowLimits(double min, double max);

		void Update(double z);

		double _minZ, _maxZ;
	};

	class VolumeMapping
	{
	public:
		PolygonalObject _bboxObj;
		std::vector<const Polygon3D*> _polygons;
	};

private:
	void PartitionIntoBBoxes(const PolygonalModel& model);

private:
	size_t _height, _width;
	std::set<ShadowEvent>* _stencil;
	std::map<size_t, ShadowLimits>* _stencil2;
	LightSource _lightSource;
	ShadowEventType _currShadowMode;
	std::vector<std::vector<VolumeMapping>> _boundingBoxes;
	int _currPolyId = 0;
	int _currModelId = 0;
	double _shadowLength;
	std::vector<std::vector<Polygon3D>> _shadowSurfaces;
	std::vector<std::vector<Normals::PolygonNormalData>> _shadowSurfaceNormals;
};

