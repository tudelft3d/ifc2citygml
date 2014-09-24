#ifndef COMMON_GEOM_GRIVY
#define COMMON_GEOM_GRIVY

#include "stdafx.h"
#include "ManifoldFix.h"

Nef_polyhedron convexFix(Nef_polyhedron& nefPolyhe, double tapeSize);

std::list<Triangle_3> get_triangles(Nef_polyhedron& nefPolyhe);
std::list<Triangle_3> get_triangles(Polyhedron& polyhe);

Point_3 comp_cog(Nef_polyhedron& nefPolyhe);
Point_3 comp_cog(Polyhedron& polyhe);

Kernel::FT comp_volume(Nef_polyhedron& nefPolyhe);
Kernel::FT comp_volume(Polyhedron& polyhe);

bool normalizeVector(Vector_3& vec);
double comp_angle(Vector_3 vec1,Vector_3 vec2);

bool is_colinear(Direction_3& dir1,Direction_3& dir2);
bool is_colinear(Vector_3& normal1,Vector_3& normal2, double angleThresh=-1);
bool is_coplanar(Polyhedron::Halfedge_handle heH, bool checkSem=false) ;
bool is_NewelCoplanar(Polyhedron::Halfedge_handle heH, bool checkSem=false, double angleThresh=-1);

pointVector comp_facetPoints(Polyhedron::Facet_const_handle fh);
//pointVector comp_facetPoints(Polyhedron::Facet_handle fh);

Triangle_3 create_triangle(Polyhedron::Facet_const_handle fch);
Triangle_3 create_triangle(pointVector& facetPoints);

Kernel::FT comp_facetSquaredArea(Polyhedron::Facet_const_handle fch);
Kernel::FT comp_facetSquaredArea(pointVector& facetPoints);

Point_3 comp_facetCentroid(Polyhedron::Facet_const_handle fch);
Point_3 comp_facetCentroid(pointVector& facetPoints);

Point_3 comp_averagePoint(pointVector& pVec);
Transformation getRotationMatrix(Vector_3 sourceVec,Vector_3 targetVec);

std::vector<Nef_polyhedron> splitNefs (Nef_polyhedron N, bool doCut=DO_CUT_FIX, bool onlyExteriorShells=true);

#ifdef WIN32 
	Nef_polyhedron mergeNefs (std::vector<Nef_polyhedron>& nefPolyheVector, int iteration=0);
#else
	Nef_polyhedron mergeNefs (std::vector<Nef_polyhedron> nefPolyheVector);
#endif




// Replaces default nef constructor which doesn't always work
// Regularization is optional
// Returns true if succesfull
bool polyhe2nef (Polyhedron& polyhe, Nef_polyhedron& nefPolyhe, bool regular=true, bool normal = true, bool useDefault=false);

// Regularization is optional
// Returns true if succesfull
bool nef2polyhe (Nef_polyhedron& nefPolyhe, Polyhedron& polyhe, bool regular=true, bool normal=true);

std::pair<Kernel::Vector_3,Kernel::FT> compute_vertexNormal(const Vertex& v);
std::pair<Kernel::Vector_3,Kernel::FT> compute_nefVertexNormal(const Nef_polyhedron::Vertex& v);

Nef_polyhedron	createBBox (Nef_polyhedron& nefPolyhe);
Polyhedron		createBBox (Polyhedron& polyhe);
#endif