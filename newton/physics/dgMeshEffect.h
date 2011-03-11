/* Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __dgMeshEffect_H__
#define __dgMeshEffect_H__

#include <dgRefCounter.h>

class dgCollision;
class dgMeshTreeCSGFace;
class dgMeshEffectSolidTree;
class dgMeshTreeCSGEdgePool;
class dgMeshTreeCSGPointsPool;


#define DG_MESH_EFFECT_INITIAL_VERTEX_SIZE	8
#define DG_MESH_EFFECT_BOLLEAN_STACK		128
#define DG_MESH_EFFECT_POINT_SPLITED		512
#define DG_MESH_EFFECT_POLYGON_SPLITED		256
#define DG_MESH_EFFECT_TRIANGLE_MIN_AREA	(dgFloat32 (1.0e-9f))
//#define DG_MESH_EFFECT_PLANE_TOLERANCE		(dgFloat64 (1.0e-5f))
#define DG_MESH_EFFECT_FLAT_CUT_BORDER_EDGE	0x01
#define DG_MESH_EFFECT_QUANTIZE_FLOAT(x)	(x)
#define DG_VERTEXLIST_INDEXLIST_TOL			(dgFloat32 (1.0e-6f))





class dgMeshEffect: public dgPolyhedra, public dgRefCounter
{
	public:

	struct dgVertexAtribute 
	{
		dgVector m_vertex;
		dgTriplex m_normal;
		dgFloat32 m_u0;
		dgFloat32 m_v0;
		dgFloat32 m_u1;
		dgFloat32 m_v1;
		dgInt32 m_material;
	};

	struct dgIndexArray 
	{
		dgInt32 m_materialCount;
		dgInt32 m_indexCount;
		dgInt32 m_materials[256];
		dgInt32 m_materialsIndexCount[256];
		dgInt32* m_indexList;
	};


	dgMeshEffect(dgMemoryAllocator* const allocator, bool preAllocaBuffers);
	dgMeshEffect(dgCollision* const collision);
	dgMeshEffect(const dgMeshEffect& source);
	dgMeshEffect(dgPolyhedra& mesh, const dgMeshEffect& source);
	
	// Create a convex hull Mesh form point cloud
	dgMeshEffect (dgMemoryAllocator* const allocator, const dgFloat32* const vertexCloud, dgInt32 count, dgInt32 strideInByte, dgFloat32 distTol);

	// create a planar Mesh
	dgMeshEffect(dgMemoryAllocator* const allocator, const dgMatrix& planeMatrix, dgFloat32 witdth, dgFloat32 breadth, dgInt32 material, const dgMatrix& textureMatrix0, const dgMatrix& textureMatrix1);
	virtual ~dgMeshEffect(void);

	dgMatrix CalculateOOBB (dgVector& size) const;
	void CalculateAABB (dgVector& min, dgVector& max) const;

	void CalculateNormals (dgFloat32 angleInRadians);
	void SphericalMapping (dgInt32 material);
	void BoxMapping (dgInt32 front, dgInt32 side, dgInt32 top);
	void CylindricalMapping (dgInt32 cylinderMaterial, dgInt32 capMaterial);

	dgEdge* InsertEdgeVertex (dgEdge* const edge, dgFloat32 param);

	dgMeshEffect* Union (const dgMatrix& matrix, const dgMeshEffect* clip) const;
	dgMeshEffect* Difference (const dgMatrix& matrix, const dgMeshEffect* clip) const;
	dgMeshEffect* Intersection (const dgMatrix& matrix, const dgMeshEffect* clip) const;
	void ClipMesh (const dgMatrix& matrix, const dgMeshEffect* clip, dgMeshEffect** top, dgMeshEffect** bottom) const;

	bool CheckIntersection (const dgMeshEffectSolidTree* const solidTree, dgFloat32 scale) const;
	dgMeshEffectSolidTree* CreateSolidTree() const;
	static void DestroySolidTree (dgMeshEffectSolidTree* tree);
	static bool CheckIntersection (const dgMeshEffect* const meshA, const dgMeshEffectSolidTree* const solidTreeA,
								   const dgMeshEffect* const meshB, const dgMeshEffectSolidTree* const solidTreeB, dgFloat32 scale);

	void Triangulate ();
	void ConvertToPolygons ();

	void RemoveUnusedVertices(dgInt32* const vertexRemapTable);
	
	void BeginPolygon ();
	void AddPolygon (dgInt32 count, const dgFloat32* vertexList, dgInt32 stride, dgInt32 material);
	void EndPolygon ();

	void PackVertexArrays ();

	void BuildFromVertexListIndexList(dgInt32 faceCount, const dgInt32 * const faceIndexCount, const dgInt32 * const faceMaterialIndex, 
		const dgFloat32* const vertex, dgInt32  vertexStrideInBytes, const dgInt32 * const vertexIndex,
		const dgFloat32* const normal, dgInt32  normalStrideInBytes, const dgInt32 * const normalIndex,
		const dgFloat32* const uv0, dgInt32  uv0StrideInBytes, const dgInt32 * const uv0Index,
		const dgFloat32* const uv1, dgInt32  uv1StrideInBytes, const dgInt32 * const uv1Index);


	dgInt32 GetVertexCount() const;
	dgInt32 GetVertexStrideInByte() const {return sizeof (dgVector);}
	dgFloat32* GetVertexPool () const {return &m_points[0].m_x;}

	dgInt32 GetPropertiesCount() const;
	dgInt32 GetPropertiesStrideInByte() const {return sizeof (dgVertexAtribute);}
	dgFloat32* GetAttributePool() const {return &m_attib->m_vertex.m_x;}
	dgFloat32* GetNormalPool() const {return &m_attib->m_normal.m_x;}
	dgFloat32* GetUV0Pool() const {return &m_attib->m_u0;}
	dgFloat32* GetUV1Pool() const {return &m_attib->m_u1;}

	dgEdge* ConectVertex (dgEdge* const e0, dgEdge* const e1);

	dgInt32 GetTotalFaceCount() const;
	dgInt32 GetTotalIndexCount() const;
	void GetFaces (dgInt32* const faceCount, dgInt32* const materials, void** const faceNodeList) const;

	void WeldTJoints ();
	bool SeparateDuplicateLoops (dgEdge* const edge);
	bool HasOpenEdges () const;
	dgFloat32 CalculateVolume () const;

	void GetVertexStreams (dgInt32 vetexStrideInByte, dgFloat32* vertex, dgInt32 normalStrideInByte, dgFloat32* normal, dgInt32 uvStrideInByte0, dgFloat32* uv0, dgInt32 uvStrideInByte1, dgFloat32* uv1);
	void GetIndirectVertexStreams(dgInt32 vetexStrideInByte, dgFloat32* vertex, dgInt32* vertexIndices, dgInt32* vertexCount,
								  dgInt32 normalStrideInByte, dgFloat32* normal, dgInt32* normalIndices, dgInt32* normalCount,
								  dgInt32 uvStrideInByte0, dgFloat32* uv0, dgInt32* uvIndices0, dgInt32* uvCount0,
								  dgInt32 uvStrideInByte1, dgFloat32* uv1, dgInt32* uvIndices1, dgInt32* uvCount1);

	

	dgIndexArray* MaterialGeomteryBegin();
	void MaterialGeomteryEnd(dgIndexArray* handle);
	dgInt32 GetFirstMaterial (dgIndexArray* Handle);
	dgInt32 GetNextMaterial (dgIndexArray* handle, dgInt32 materialHandle);
	dgInt32 GetMaterialID (dgIndexArray* handle, dgInt32 materialHandle);
	dgInt32 GetMaterialIndexCount (dgIndexArray* handle, dgInt32 materialHandle);
	void GetMaterialGetIndexStream (dgIndexArray* handle, dgInt32 materialHandle, dgInt32* index);
	void GetMaterialGetIndexStreamShort (dgIndexArray* handle, dgInt32 materialHandle, dgInt16* index);

	dgCollision* CreateConvexApproximationCollision(dgWorld* const world, dgInt32 maxCount, dgInt32 shapeId, dgInt32 childrenID) const;
	dgCollision* CreateConvexCollision(dgFloat32 tolerance, dgInt32 shapeID, const dgMatrix& matrix = dgGetIdentityMatrix()) const;
	
	dgMeshEffect* CreateVoronoiPartition (dgInt32 pointsCount, dgInt32 pointStrideInBytes, const dgFloat32* const pointCloud, dgInt32 interionMaterial) const;

	void PlaneClipMesh (const dgPlane& plane, dgMeshEffect** leftMeshSource, dgMeshEffect** rightMeshSource);

	dgVertexAtribute& GetAttribute (dgInt32 index) const;
	void TransformMesh (const dgMatrix& matrix);


	void* GetFirstVertex ();
	void* GetNextVertex (const void* vertex);
	int GetVertexIndex (const void* vertex) const;

	void* GetFirstPoint ();
	void* GetNextPoint (const void* point);
	int GetPointIndex (const void* point) const;
	int GetVertexIndexFromPoint (const void* point) const;


	void* GetFirstEdge ();
	void* GetNextEdge (const void* edge);
	void GetEdgeIndex (const void* edge, dgInt32& v0, dgInt32& v1) const;
//	void GetEdgeAttributeIndex (const void* edge, dgInt32& v0, dgInt32& v1) const;

	void* GetFirstFace ();
	void* GetNextFace (const void* face);
	int IsFaceOpen (const void* face) const;
	int GetFaceMaterial (const void* face) const;
	int GetFaceIndexCount (const void* face) const;
	void GetFaceIndex (const void* face, int* indices) const;
	void GetFaceAttributeIndex (const void* face, int* indices) const;

	bool Sanity () const;
	private:

	void Init (bool preAllocaBuffers);
	dgVector GetOrigin ()const;
	dgInt32 CalculateMaxAttributes () const;
	void EnumerateAttributeArray (dgVertexAtribute* attib);
	void ApplyAttributeArray (dgVertexAtribute* attib);
	void AddVertex(const dgVector& vertex);
	void AddAtribute (const dgVertexAtribute& attib);
	void AddPoint(const dgFloat32* vertexList, dgInt32 material);
	void FixCylindricalMapping (dgVertexAtribute* attib) const;

	void MergeFaces (dgMeshEffect* const source);
	void ReverseMergeFaces (dgMeshEffect* const source);
	dgVertexAtribute InterpolateEdge (dgEdge* const edge, dgFloat32 param) const;
	dgVertexAtribute InterpolateVertex (const dgVector& point, dgEdge* const face) const;
	
	void ClipMesh (const dgMeshEffect* clipMesh, dgMeshEffect** leftMeshSource, dgMeshEffect** rightMeshSource) const;
	void ClipMesh (const dgMeshEffectSolidTree* const clipper, dgMeshEffect** leftMeshSource, dgMeshEffect** rightMeshSource) const;
	dgInt32 PlaneApplyCap (const dgMeshEffect* planeMesh, const dgPlane& normal);
	void PlaneClipMesh (const dgMeshEffect* planeMesh, dgMeshEffect** leftMeshSource, dgMeshEffect** rightMeshSource) const;

	void CopyCGSFace (const dgMeshEffect& reference, dgEdge* const face);
	void AddCGSFace (const dgMeshEffect& reference, dgEdge* const refFace, dgInt32 count, dgMeshTreeCSGFace** const faces, const dgMeshTreeCSGPointsPool& points);
//	void ClipFace (const dgBigPlane& plane, dgMeshTreeCSGFace* src, dgMeshTreeCSGFace** left, dgMeshTreeCSGFace** right,	dgMeshTreeCSGPointsPool& pointPool) const;
	void ClipFace (const dgHugeVector& normal, const dgHugeVector& origin, dgMeshTreeCSGFace* const src, dgMeshTreeCSGFace** left, dgMeshTreeCSGFace** const right, dgMeshTreeCSGPointsPool& pointPool) const;

	bool CheckSingleMesh() const;


	dgInt32 m_isFlagFace;
	dgInt32 m_pointCount;
	dgInt32 m_maxPointCount;

	dgInt32 m_atribCount;
	dgInt32 m_maxAtribCount;

	dgVector* m_points;
	dgVertexAtribute* m_attib;

	
	friend class dgConvexHull3d;
	friend class dgConvexHull4d;
	friend class dgMeshEffectSolidTree;
};

class dgMeshEffectSolidTree
{
	public:

	class CSGConvexCurve: public dgList<dgHugeVector>
	{
		public:

		CSGConvexCurve ()
			:dgList<dgHugeVector>(NULL)
		{

		}

		CSGConvexCurve (dgMemoryAllocator* const allocator)
			:dgList<dgHugeVector>(allocator)
		{
		}

		//bool CheckConvex(const dgBigVector& plane) const
		bool CheckConvex(const dgHugeVector& normal, const dgHugeVector& point) const
		{
//			dgBigVector p1 (GetLast()->GetInfo());
//			dgBigVector p0 (GetLast()->GetPrev()->GetInfo());
//			dgBigVector e0 (p0 - p1);
//			for (CSGConvexCurve::dgListNode* node = GetFirst(); node; node = node->GetNext()) {
//				dgFloat64 convex;
//				dgBigVector p2 (node->GetInfo());
//				dgBigVector e1 (p2 - p1);
//
//				dgBigVector n (e1 * e0);
//				convex = n % plane;
//				if (convex < dgFloat64 (-1.0e5f)) {
//					return false;
//				}
//				p1 = p2;
//				e0 = e1.Scale (-1.0f);
//			}

			return true;
		}
	};

	DG_CLASS_ALLOCATOR(allocator)

	dgMeshEffectSolidTree (const dgMeshEffect& mesh, dgEdge* const face)
	{
		BuildPlane (mesh, face, m_normal, m_point);
//		normal = normal.Scale (1.0f / sqrt (normal % normal));
//		m_plane = dgBigPlane (normal, - (normal % origin));
		m_front = NULL;
		m_back = NULL;
	}

	dgMeshEffectSolidTree (const dgHugeVector& normal, const dgHugeVector& point)
		:m_point (point), m_normal (normal)
	{
		m_front = NULL;
		m_back = NULL;
	}


	~dgMeshEffectSolidTree()
	{
		if (m_front)	{
			delete m_front; 
		}

		if (m_back)	{
			delete m_back;
		}
	}

	void BuildPlane (const dgMeshEffect& mesh, dgEdge* const face, dgHugeVector& normal, dgHugeVector& point) const
	{
		point = dgHugeVector(dgBigVector (mesh.m_points[face->m_incidentVertex]));
		dgEdge* edge = face;
		//dgHugeVector p0 (dgBigVector (&pool[edge->m_incidentVertex * stride]));
		normal = dgHugeVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
		dgHugeVector p0 (dgBigVector (mesh.m_points[edge->m_incidentVertex]));
		edge = edge->m_next;
		//dgBigVector p1 (&pool[edge->m_incidentVertex * stride]);
		dgHugeVector p1 (dgBigVector (mesh.m_points[edge->m_incidentVertex]));
		dgHugeVector e1 (p1 - p0);
		for (edge = edge->m_next; edge != face; edge = edge->m_next) {
			//dgBigVector p2 (&pool[edge->m_incidentVertex * stride]);
			dgHugeVector p2 (dgBigVector (mesh.m_points[edge->m_incidentVertex]));
			dgHugeVector e2 (p2 - p0);
			normal += e1 * e2;
			e1 = e2;
		} 
	}
	

	void AddFace (const dgMeshEffect& mesh, dgEdge* const face)
	{
//		dgBigPlane plane (MakePlane (mesh, face));
		dgBigVector normal (mesh.FaceNormal (face, &mesh.m_points[0][0], sizeof (dgVector)));
		dgFloat64 mag2 = normal % normal;

		if (mag2 > dgFloat32 (1.0e-14f)) {

			CSGConvexCurve faces[DG_MESH_EFFECT_BOLLEAN_STACK];
			dgMeshEffectSolidTree* pool[DG_MESH_EFFECT_BOLLEAN_STACK];

//			dgBigVector origin (mesh.m_points[face->m_incidentVertex]);
//			normal = normal.Scale (1.0f / sqrt (mag2));
//			dgBigPlane plane (normal, - (normal % origin));
			dgHugeVector point;
			dgHugeVector normal;
			BuildPlane (mesh, face, normal, point);

			dgEdge* ptr = face;
			faces[0].SetAllocator(mesh.GetAllocator());
			do {
				faces[0].Append(dgBigVector (mesh.m_points[ptr->m_incidentVertex]));
				ptr = ptr->m_next;
			} while (ptr != face);

			dgInt32 stack = 1;
			pool[0] = this;
			while (stack) {

				stack --;
				dgMeshEffectSolidTree* const root = pool[stack];
				CSGConvexCurve& curve = faces[stack];
				_ASSERTE (curve.CheckConvex(normal, point));

				dgGoogol minDist (dgFloat64 (0.0f));
				dgGoogol maxDist (dgFloat64 (0.0f));
				for (CSGConvexCurve::dgListNode* node = curve.GetFirst(); node; node = node->GetNext()) {
					//dist = root->m_plane.Evalue(node->GetInfo());
					dgGoogol dist = root->m_normal % (node->GetInfo() - root->m_point);
//					if (dist < - DG_MESH_EFFECT_PLANE_TOLERANCE) {
					if (dist.GetAproximateValue() < dgFloat32 (0.0f)) {
						minDist = dist;
					}
//					if (dist > DG_MESH_EFFECT_PLANE_TOLERANCE) {
					if (dist.GetAproximateValue() > dgFloat32 (0.0f)) {
						maxDist = dist;
					} 
				} 

				if ((minDist.GetAproximateValue() < dgFloat64 (0.0f)) && (maxDist.GetAproximateValue() > dgFloat64 (0.0f))) {
					_ASSERTE (0);
					CSGConvexCurve tmp(mesh.GetAllocator());
					for (CSGConvexCurve::dgListNode* node = curve.GetFirst(); node; node = node->GetNext()) {
						tmp.Append(node->GetInfo());
					}
					curve.RemoveAll();
				
					if (!root->m_back) {
						root->m_back = new (mesh.GetAllocator()) dgMeshEffectSolidTree (normal, point);
					} else {
/*
						dgFloat64 test0;
						dgBigVector p0 (tmp.GetLast()->GetInfo());
						CSGConvexCurve& backFace = faces[stack];
					
						backFace.SetAllocator(mesh.GetAllocator());
						test0 = root->m_plane.Evalue(p0);
						for (CSGConvexCurve::dgListNode* node = tmp.GetFirst(); node; node = node->GetNext()) {
							dgFloat64 test1;

							dgBigVector p1 (node->GetInfo());
							test1 = root->m_plane.Evalue(p1);
							if (test0 <= dgFloat64 (0.0f)) {
								backFace.Append(p0);
								if (test0 < dgFloat64 (0.0f) && (test1 > dgFloat64 (0.0f))) {
									dgFloat64 den;

									dgBigVector dp (p1 - p0);
									den = root->m_plane % dp;
									dgBigVector p (p0 + dp.Scale (-test0 / den));

									backFace.Append(p);
								}
							} else if (test1 < dgFloat64 (0.0f)) {
								dgFloat64 den;

								dgBigVector dp (p1 - p0);
								den = root->m_plane % dp;
								dgBigVector p (p0 + dp.Scale (-test0 / den));

								backFace.Append(p);
							}

							test0 = test1;
							p0 = p1;
						}

						// check Here because the clipper can generate a point and lines
						_ASSERTE (!backFace.GetCount() || (backFace.GetCount() >= 3));

						pool[stack] = root->m_back;
						stack ++;
						_ASSERTE (stack < (sizeof (pool)/sizeof (pool[0])));
*/
					}

/*					
					if (!root->m_front) {
						root->m_front = new (mesh.GetAllocator())dgMeshEffectSolidTree (plane);
					} else {

						dgFloat64 test0;
						dgBigVector p0 (tmp.GetLast()->GetInfo());
						CSGConvexCurve& frontFace = faces[stack];

						frontFace.SetAllocator(mesh.GetAllocator());
						test0 = root->m_plane.Evalue(p0);
						for (CSGConvexCurve::dgListNode* node = tmp.GetFirst(); node; node = node->GetNext()) {
							dgFloat64 test1;

							dgBigVector p1 (node->GetInfo());
							test1 = root->m_plane.Evalue(p1);
							if (test0 >= dgFloat64 (0.0f)) {
								frontFace.Append(p0);
								if (test0 > dgFloat64 (0.0f) && (test1 < dgFloat32 (0.0f))) {
									dgFloat64 den;

									dgBigVector dp (p1 - p0);
									den = root->m_plane % dp;
									dgBigVector p (p0 + dp.Scale (-test0 / den));
									frontFace.Append(p);
								}
							} else if (test1 > dgFloat64 (0.0f)) {
								dgFloat64 den;

								dgBigVector dp (p1 - p0);
								den = root->m_plane % dp;
								dgBigVector p (p0 + dp.Scale (-test0 / den));

								frontFace.Append(p);
							}

							test0 = test1;
							p0 = p1;
						}

						// check Here because the clipper can generate a point and lines
						_ASSERTE (!frontFace.GetCount() || (frontFace.GetCount() >= 3));

						pool[stack] = root->m_front;
						stack ++;
						_ASSERTE (stack < (sizeof (pool)/sizeof (pool[0])));
					}

*/
				} else {

					if (minDist.GetAproximateValue() < dgFloat64 (0.0f)) {
						if (!root->m_back) {
							root->m_back = new (mesh.GetAllocator())dgMeshEffectSolidTree (normal, point);
						} else {
							pool[stack] = root->m_back;
							stack ++;
							_ASSERTE (stack < (sizeof (pool)/sizeof (pool[0])));
						}
						
					} else if (maxDist.GetAproximateValue() > dgFloat64 (0.0f)) {
						if (!root->m_front) {
							root->m_front = new (mesh.GetAllocator())dgMeshEffectSolidTree (normal, point);
						} else {
							pool[stack] = root->m_front;
							stack ++;
							_ASSERTE (stack < (sizeof (pool)/sizeof (pool[0])));
						}
					}
				}
			}
		}
	}

//	dgBigPlane m_plane;
	dgHugeVector m_point;
	dgHugeVector m_normal;
	dgMeshEffectSolidTree* m_back;
	dgMeshEffectSolidTree* m_front;
};


inline dgInt32 dgMeshEffect::GetVertexCount() const
{
	return m_pointCount;
}

inline dgInt32 dgMeshEffect::GetPropertiesCount() const
{
	return m_atribCount;
}

inline dgInt32 dgMeshEffect::GetMaterialID (dgIndexArray* handle, dgInt32 materialHandle)
{
	return handle->m_materials[materialHandle];
}

inline dgInt32 dgMeshEffect::GetMaterialIndexCount (dgIndexArray* handle, dgInt32 materialHandle)
{
	return handle->m_materialsIndexCount[materialHandle];
}

inline dgMeshEffect::dgVertexAtribute& dgMeshEffect::GetAttribute (dgInt32 index) const 
{
	return m_attib[index];
}

#endif
