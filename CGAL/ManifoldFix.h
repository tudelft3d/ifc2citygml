#ifndef MANIFOLDFIX_GRIVY
#define MANIFOLDFIX_GRIVY

#include "stdafx.h"
#include "CommonGeomFunctions.h"
#include "Minkowski.h"

/* ------------------ Local Typedefs ------------------*/
typedef std::pair<Polyhedron::Halfedge_handle,Polyhedron::Halfedge_handle> HalfedgePair;
typedef std::pair<Kernel::FT,HalfedgePair> SortDegPairs;
// ---- //
typedef Nef_polyhedron::SNC_structure							SNC_structure;
typedef SNC_structure::Halfedge_const_handle					Halfedge_const_handle;
typedef SNC_structure::Vertex_const_handle						Vertex_const_handle;
typedef SNC_structure::Sphere_map								Sphere_map;
typedef CGAL::SM_decorator<Sphere_map>							SM_decorator;
typedef SM_decorator::SFace_const_iterator						SFace_const_iterator;

/* ------------------ Manifold Fix Functions ------------------*/
bool make2Manifold(Nef_polyhedron& nefPolyh, double fixSize=-1, bool doCut=DO_CUT_FIX);//, Vector_3 commonDir=CGAL::NULL_VECTOR);
bool is_edge_2manifold(const Halfedge_const_handle& e);
bool is_vertex_2manifold(const Vertex_const_handle& v);

/* ------------------ Other Function ------------------*/
std::pair<Vector_3,Vector_3> getMostCommonVector(Polyhedron& polyhe, bool NOzAxis=true);
void round_Vertex_3(Polyhedron::Vertex_handle& vrt);
Point_3 round_Point_3(Point_3& pnt);
bool roundCoordsSafely (Polyhedron& exteriorPolyhe,std::vector<Nef_polyhedron>& nefVec=std::vector<Nef_polyhedron>());

/* ------------------ Degen Fix Functions ------------------*/
SortDegPairs sort_degenTri(Polyhedron::Facet_handle& fch);
void fix_degenerate(Polyhedron& polyhe);
bool triangleIsDegenerate(pointVector& triPoints);
bool triangleIsDegenerate(Polyhedron::Facet_handle& fch);
bool do_removeCap(Polyhedron& polyhe,Polyhedron::Halfedge_handle longHE,const Kernel::FT& lengthThreshold);
bool do_edgeCollapse(Polyhedron& poly, Polyhedron::Halfedge_handle h);
bool do_edgeFlip(Polyhedron& polyhe, Polyhedron::Halfedge_handle h, bool checkDegen=false);




/* TEST */
/*
	std::ifstream iftstrm("c:\\dropbox\\coding\\libraries\\grivydev\\src\\shapes\\NonManifoldEdge.off");
	Nef_polyhedron n;
	Polyhedron p;	
	
	get_unitBox(n);
	n.transform(getRotationMatrix(Vector_3(0,0,1),Vector_3(0.3,0.55,0)));
	n.convert_to_polyhedron(p);

	n.clear();
	CGAL::OFF_to_nef_3(iftstrm,n);
	n.transform(getRotationMatrix(Vector_3(0,0,1),Vector_3(0.3,0.55,0)));

	Vector_3 commonDir = getMostCommonVector(p).first;
	std::cout << commonDir << std::endl;

	make2Manifold(n,commonDir);
	write_nef2off(n,"C:\\Dropbox\\Coding\\Libraries\\grivyDEV\\src\\Shapes\\NonManifoldEdge_improvedfix");
*/
#endif