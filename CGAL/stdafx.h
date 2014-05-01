#ifndef STDAFX_GRIVY
#define STDAFX_GRIVY

// Compile-time settings

#define MANIFOLD_FIRST				true		// Set to false if "Nef_polyhedron(N,si)" WORKS on non-manifolds and profit!	
#define MANIFOLD_FIRST_EXTERIOR		true		// Set to false if "Nef_polyhedron(N,si)" WORKS on non-manifolds and profit!	
#define DO_CUT_FIX					false		// True-> cut box to fix non-manifolds (in exterior shell only)
#define MANIFOLD_CUBE_FIX						// comment to use tailor-made element thing

#define DEFAULT_UP_SEMANTIC			"Floor"
#define DEFAULT_HOR_SEMANTIC		"Wall"
#define DEFAULT_DOWN_SEMANTIC		"Ceiling"
#define TO_DIST_SEMANTIC			"ToDist"

#define OUTPUT_DECIMALS				10 // 10

#define HORIZONTAL_ANGLE_RANGE		0.08715574	// sin(5deg)

#define OVERLAP_DIST_THRESHOLD		1e-4		// squared distance Region which is considered Equidistant
#define OVERLAP_ANGLE_THRESHOLD		0.17453293	// 10deg
#define OVERLAP_AREA_THRESHOLD		1			// m2

#define SEMANTIC_DISTANCE_THRESHOLD	1e-4		// If SQUARED distance is larger the default semantic will be assigned/"Anything"
#define MIN_ANGLE_THRESHOLD			0.0001		// Rad,Minimal squared triangle area (0.0001)
#define MIN_LENGTH_THRESHOLD		1e-3		// m, Minimal edge length

#define MAX_VALUE					999999999	// max value..
#define SSTREAM_PRECISION			50			// internal stringstream.precision( ## )

#define OFFSET_MAGNITUDE			150			// DEFAULT Size of Minkowski robot for closing operation

#define FIX_MANIFOLD_SIZE			0.01		// 0.01 for cube fix
#define MANIFIX_MAX_ITR				2000		// Maximum number of fixes to be applied when fixing non-manifold

#include <fstream>
#include <sstream> 
#include <algorithm>

/* ------------------- Kernel --------------------------------*/
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>


/* ------------------- Polyhedron ----------------------------*/
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>

/* ------------------- NEF -----------------------------------*/
#include <CGAL/Nef_polyhedron_3.h>

#include <CGAL/IO/Nef_polyhedron_iostream_3.h>
#include <CGAL/OFF_to_nef_3.h>
#include <CGAL/Nef_nary_union_3.h>

//new
#include <CGAL/minkowski_sum_3.h>
#include <CGAL/Nef_3/SNC_indexed_items.h>

#include <CGAL/convex_decomposition_3.h> 
#include <CGAL/convex_hull_3.h>
#include <CGAL/bounding_box.h>
#include <CGAL/Bbox_3.h>

#include <CGAL/centroid.h>

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_polyhedron_triangle_primitive.h>

template <class Refs, class Plane, class Traits>
struct My_face : public CGAL::HalfedgeDS_face_base<Refs, CGAL::Tag_true, Plane>{
	//extended property for a face
	typedef typename Traits::Point_3 Point_3;
	typedef typename Traits::FT FT;
	double area;

	FT leastSqDistance;
	std::string semanticBLA;
	std::vector<std::string> equidistSems;
	bool isMinkFacet;
	My_face():leastSqDistance(MAX_VALUE){}
	My_face(std::string sem) : semanticBLA(sem){}
};
struct polItems : public CGAL::Polyhedron_items_3{
	template<class Refs,class Traits>
	struct Face_wrapper{
		typedef typename Traits::Plane_3 Plane;
		typedef My_face<Refs, Plane, Traits> Face;
	};
}; 

/* ------------------- Kernel --------------------------------*/
typedef CGAL::Exact_predicates_exact_constructions_kernel				Kernel;

/* ------------------- Geometry ------------------------------*/

typedef Kernel::Point_3													Point_3;
typedef Kernel::Direction_3												Direction_3;
typedef Kernel::Vector_3												Vector_3;
typedef Kernel::Ray_3												Ray_3;
typedef CGAL::Line_3<Kernel>											Line_3;
typedef CGAL::Segment_3<Kernel>											Segment_3;
typedef Kernel::Plane_3													Plane_3;
typedef CGAL::Triangle_3<Kernel>										Triangle_3;
typedef Kernel::Iso_cuboid_3											Bounding_box;

/* ------------------- NEF -----------------------------------*/
typedef CGAL::Nef_polyhedron_3<Kernel>			Nef_polyhedron;//_SNC;  , CGAL::SNC_indexed_items

typedef Nef_polyhedron::Volume_const_iterator							Volume_const_iterator; //_SNC
typedef CGAL::Nef_nary_union_3<Nef_polyhedron>							NefNaryUnion;

/* ------------------- Polyhedron ----------------------------*/
typedef CGAL::Polyhedron_3<Kernel,polItems>								Polyhedron;//Pol3Itm;

typedef Polyhedron::Vertex												Vertex;
typedef Polyhedron::HalfedgeDS											HalfedgeDS;
typedef CGAL::Polyhedron_incremental_builder_3<HalfedgeDS>				polyIncrBuilder;

/* ------------------- AABB tree -----------------------------*/
typedef CGAL::AABB_polyhedron_triangle_primitive<Kernel,Polyhedron> Primitive;
typedef CGAL::AABB_traits<Kernel, Primitive> AAbbTraits;
typedef CGAL::AABB_tree<AAbbTraits> AAbbTree;
/* ------------------- Transform -----------------------------*/
typedef CGAL::Aff_transformation_3<Kernel>								Transformation;


/* ------------------- Vectors & Pairs -----------------------*/
typedef std::vector<Point_3>											pointVector;
typedef std::vector<Nef_polyhedron>										nefVector;
typedef std::pair<std::vector<std::string>,Nef_polyhedron>				nefPair;
typedef std::vector<nefPair>											nefPairVector;
typedef std::vector<Polyhedron>											PolVector;
typedef std::vector<std::vector<int>>									facesVector;
typedef std::pair<std::vector<std::string>,std::vector<int>>			offxPair;
typedef std::vector<offxPair>											offxPairVector;

//typedef std::map<Point_3,Point_3>										newoldPointMap;

typedef std::pair<std::string,std::set<Polyhedron::Facet_handle>>		sfsPair;
typedef std::vector<sfsPair>											sfsVec; // Semantic Facet Sets Vector

/* ------------------- Iterators -----------------------------*/
typedef offxPairVector::iterator										opvItr;

typedef std::vector<std::set<std::string>>								inexcludeMap;

struct Plane_equation {	// only for convex facets
    template <class Facet>
    typename Facet::Plane_3 operator()( Facet& f) {
        typename Facet::Halfedge_handle h = f.halfedge();
        typedef typename Facet::Plane_3  Plane;
        return Plane( h->vertex()->point(),
                      h->next()->vertex()->point(),
                      h->next()->next()->vertex()->point());
    }
};// should use newel instead

struct Plane_Newel_equation { // for all facets
	 template <class Facet>
    typename Facet::Plane_3 operator()( Facet& f) {
        typedef typename Facet::Plane_3   Plane;
		pointVector facetPoints;
		typename Facet::Halfedge_around_facet_const_circulator itHDS = f.facet_begin();
		do facetPoints.push_back(itHDS->vertex()->point());				// Get vertexes of face
		while (++itHDS != f.facet_begin());
		Vector_3  ortVec;
		CGAL::normal_vector_newell_3(facetPoints.begin(),facetPoints.end(),ortVec);
        return Plane(facetPoints[0],ortVec);
    }
};



#endif