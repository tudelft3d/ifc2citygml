#include "CommonGeomFunctions.h"

Nef_polyhedron convexFix(Nef_polyhedron& nefPolyhe, double tapeSize) {
	pointVector pnts;
	for(Nef_polyhedron::Vertex_const_iterator vIt=nefPolyhe.vertices_begin();vIt!=nefPolyhe.vertices_end();++vIt) {
		pnts.push_back(vIt->point());
		pnts.push_back(Point_3(vIt->point().x()+tapeSize,vIt->point().y()+tapeSize,vIt->point().z()+tapeSize));
	}
	Polyhedron polyHull;
	CGAL::convex_hull_3(pnts.begin(),pnts.end(),polyHull);
	return Nef_polyhedron(polyHull);
}

// Doeas not work
std::list<Triangle_3> get_triangles(Nef_polyhedron& nefPolyhe) {
	std::list<Triangle_3> triangles;
	pointVector pnts;
	
	Nef_polyhedron::Halffacet_const_iterator fit;
	CGAL_forall_facets(fit,nefPolyhe) {
		Nef_polyhedron::Halffacet_cycle_const_iterator cycle;
		CGAL_forall_facet_cycles_of(cycle, fit) {
			if (cycle.is_shalfedge()) {	// Only consider non-trivial cycles.	
				Nef_polyhedron::SHalfedge_const_handle half_edge = cycle;
				//half_edge->in_inner_facet_cycle()
				//Point_3 point = half_edge->source()->point();
				
				Nef_polyhedron::SHalfedge_around_facet_const_circulator circulator(half_edge);
				Nef_polyhedron::SHalfedge_around_facet_const_circulator end(circulator);
				CGAL_For_all(circulator, end) {					        // Iterate over vertices of cycle.
					Point_3 point = circulator->source()->source()->point();
					pnts.push_back(point);
					std::cout << point << " - ";
				}
			}
			if ((int)pnts.size()!=3) std::cerr << "WARNING: Nef Facet not a triangle: "<<pnts.size() <<std::endl;
			triangles.push_back(Triangle_3(pnts[0],pnts[1],pnts[2]));
			pnts.clear();
		}
	}
	return triangles;
}

std::list<Triangle_3> get_triangles(Polyhedron& polyhe) {
	   std::list<Triangle_3> triangles;	// get triangles from the mesh
    Polyhedron::Facet_iterator f;
    for(f=polyhe.facets_begin();f!=polyhe.facets_end();++f) {
      const Point_3& a = f->halfedge()->vertex()->point();
      const Point_3& b = f->halfedge()->next()->vertex()->point();
      const Point_3& c = f->halfedge()->prev()->vertex()->point();
      triangles.push_back(Triangle_3(a,b,c));
    }
	return triangles;
}

// Not sure whether this worked..
Point_3 comp_cog(Nef_polyhedron& nefPolyhe) {
	    
    std::list<Triangle_3> triangles = get_triangles(nefPolyhe);
    //// fit plane to triangles
    //Plane plane;
    //std::cout << "Fit plane...";
    //CGAL::linear_least_squares_fitting_3(triangles.begin(),triangles.end(),plane,CGAL::Dimension_tag<2>());
    //std::cout << "ok" << std::endl;

	if ((int)triangles.size()<1) {std::cerr << "ERROR: Empty triangle list" << std::endl;return CGAL::ORIGIN;}
    // compute centroid
	Vector_3 v = CGAL::NULL_VECTOR;
	Kernel::FT sum_areas = 0;
	for(std::list<Triangle_3>::iterator it = triangles.begin(); it != triangles.end(); it++) {
		const Triangle_3& triangle = *it;
		Kernel::FT unsigned_area = std::sqrt(CGAL::to_double(triangle.squared_area()));
		Point_3 c = Kernel().construct_centroid_3_object()(triangle[0],triangle[1],triangle[2]);
		v = v + unsigned_area * (c - CGAL::ORIGIN);
		sum_areas += unsigned_area;
	}
	CGAL_assertion(sum_areas != 0.0);
	return CGAL::ORIGIN + v / sum_areas;
}

Point_3 comp_cog(Polyhedron& polyhe) {
	    
    std::list<Triangle_3> triangles = get_triangles(polyhe);

    //// fit plane to triangles
    //Plane plane;
    //std::cout << "Fit plane...";
    //CGAL::linear_least_squares_fitting_3(triangles.begin(),triangles.end(),plane,CGAL::Dimension_tag<2>());
    //std::cout << "ok" << std::endl;

	if ((int)triangles.size()<1) {std::cerr << "ERROR: Empty triangle list" << std::endl;return CGAL::ORIGIN;}
    // compute centroid
	Vector_3 v = CGAL::NULL_VECTOR;
	Kernel::FT sum_areas = 0;
	for(std::list<Triangle_3>::iterator it = triangles.begin(); it != triangles.end(); it++) {
		const Triangle_3& triangle = *it;
		Kernel::FT unsigned_area = std::sqrt(abs(CGAL::to_double(triangle.squared_area())));
		Point_3 c = Kernel().construct_centroid_3_object()(triangle[0],triangle[1],triangle[2]);
		v = v + unsigned_area * (c - CGAL::ORIGIN);
		sum_areas += unsigned_area;
	}
	CGAL_assertion(sum_areas != 0.0);
	return CGAL::ORIGIN + v / sum_areas;
}

// Not sure whether this worked..
Kernel::FT comp_volume(Nef_polyhedron& nefPolyhe) {
	Kernel::FT volume = 0;
	Point_3 p0 = CGAL::ORIGIN;
	std::list<Triangle_3> triangles = get_triangles(nefPolyhe);
	for (std::list<Triangle_3>::iterator triIt=triangles.begin();triIt!=triangles.end();++triIt) {
		volume += CGAL::volume(p0,(*triIt)[0],(*triIt)[1],(*triIt)[2]);
	}
	return volume;
}

Kernel::FT comp_volume(Polyhedron& polyhe) {
	Kernel::FT volume = 0;
	Point_3 p0 = CGAL::ORIGIN;
	pointVector fpnts;
	Polyhedron::Facet_const_iterator fcIt = polyhe.facets_begin();
	for (;fcIt!=polyhe.facets_end();++fcIt) {
		fpnts = comp_facetPoints(fcIt);
		volume += CGAL::volume(p0,fpnts[0],fpnts[1],fpnts[2]);
	}
	return volume;
}

bool normalizeVector(Vector_3& vec) {
	double length = sqrt(CGAL::to_double(vec.squared_length()));
	if (length==0) return false;
	vec = vec/length;
	return true;
}

double comp_angle(Vector_3 vec1,Vector_3 vec2) {
	if (!normalizeVector(vec1) || !normalizeVector(vec2)) {
		std::cerr << "W";//ARNING: Vector length equals 0"<<std::endl;
		return -1;
	}
	double theta;
	double scalarProd = CGAL::to_double(vec1*vec2);
	if		(scalarProd>1)	theta = 0;
	else if (scalarProd<-1)	theta = CGAL_PI;
	else theta = abs(acos(scalarProd)); 
	return theta;
}

bool is_colinear(Direction_3& dir1,Direction_3& dir2) {
	if (dir1==dir2) return true;
	else return false;
}
bool is_colinear(Vector_3& normal1,Vector_3& normal2, double angleThresh) {
	if (angleThresh>=0)
		return comp_angle(normal1,normal2) < angleThresh;
	else
		return comp_angle(normal1,normal2) < MIN_ANGLE_THRESHOLD;
}

bool is_coplanar(Polyhedron::Halfedge_handle heH, bool checkSem) {

	if (checkSem && heH->facet()->semanticBLA != heH->opposite()->facet()->semanticBLA)
		return false;

	Vector_3 normal1 = heH->face()->plane().orthogonal_vector();
	Vector_3 normal2 = heH->opposite()->face()->plane().orthogonal_vector();
	//Direction_3 normal1 = heH->face()->plane().orthogonal_direction();
	//Direction_3 normal2 = heH->opposite()->face()->plane().orthogonal_direction();

	return is_colinear(normal1,normal2);
}
bool is_NewelCoplanar(Polyhedron::Halfedge_handle heH, bool checkSem, double angleThresh) {

	if (checkSem && heH->facet()->semanticBLA != heH->opposite()->facet()->semanticBLA)
		return false;

	pointVector facetPoints1 = comp_facetPoints(heH->facet());
	pointVector facetPoints2 = comp_facetPoints(heH->opposite()->facet());
	Vector_3 ortVec1,ortVec2;
	CGAL::normal_vector_newell_3(facetPoints1.begin(),facetPoints1.end(),ortVec1);
	CGAL::normal_vector_newell_3(facetPoints2.begin(),facetPoints2.end(),ortVec2);

	if (angleThresh>=0)
		return is_colinear(ortVec1,ortVec2, angleThresh);
	else
		return is_colinear(ortVec1,ortVec2);
}

pointVector comp_facetPoints(Polyhedron::Facet_const_handle fch) {
	pointVector facetPoints;
	Polyhedron::Facet::Halfedge_around_facet_const_circulator itHDS = fch->facet_begin();
	do facetPoints.push_back(itHDS->vertex()->point());				// Get vertexes of face
	while (++itHDS != fch->facet_begin());
	return facetPoints;
}

Triangle_3 create_triangle(Polyhedron::Facet_const_handle fch) {
	pointVector facetPoints = comp_facetPoints(fch);
	return Triangle_3(facetPoints[0],facetPoints[1],facetPoints[2]);
}
Triangle_3 create_triangle(pointVector& facetPoints) {
	return Triangle_3(facetPoints[0],facetPoints[1],facetPoints[2]);
}

Kernel::FT comp_facetSquaredArea(pointVector& facetPoints) {
	return create_triangle(facetPoints).squared_area();
}
Kernel::FT comp_facetSquaredArea(Polyhedron::Facet_const_handle fch) {
	return create_triangle(fch).squared_area();
}

Point_3 comp_facetCentroid(pointVector& facetPoints) {
	return CGAL::centroid(facetPoints.begin(), facetPoints.end(),CGAL::Dimension_tag<0>());
}
Point_3 comp_facetCentroid(Polyhedron::Facet_const_handle fch) {
	pointVector facetPoints = comp_facetPoints(fch);
	return CGAL::centroid(facetPoints.begin(), facetPoints.end(),CGAL::Dimension_tag<0>());
}

// Use facetCentroid for triangles
Point_3 comp_averagePoint(pointVector& pVec) {
	Kernel::FT xP = 0;
	Kernel::FT yP = 0;
	Kernel::FT zP = 0;
	for (pointVector::iterator pvIt=pVec.begin();pvIt!=pVec.end();pvIt++) {
		xP += pvIt->x();	yP += pvIt->y();	zP += pvIt->z();
	}
	int size = pVec.size();
	return Point_3(xP/size,yP/size,zP/size);
}

Transformation getRotationMatrix(Vector_3 sourceVec,Vector_3 targetVec) {
	sourceVec = sourceVec / sqrt(CGAL::to_double(sourceVec.squared_length()));	// Normalize vectors
	targetVec = targetVec / sqrt(CGAL::to_double(targetVec.squared_length()));

	Kernel::FT  spFT = sourceVec*targetVec;										// Scalar product
	double spDouble  = CGAL::to_double(spFT);			

	if (spDouble <= -1.0 || spDouble >= 1.0)	return Transformation(1,0,0,0,1,0,0,0,1);
	double angle = acos(spDouble);												// Calculate angle

	Vector_3 cross = CGAL::cross_product(sourceVec, targetVec);					// Cross product
	double cLen = sqrt(CGAL::to_double(cross.squared_length()));
	if (angle==0 || cLen==0)					return Transformation(1,0,0,0,1,0,0,0,1);

	cross = cross / cLen;														// Normalize rotation vector
	Kernel::FT x = cross.x(); Kernel::FT y = cross.y(); Kernel::FT z = cross.z();	
	Kernel::FT co = (1-spFT);
	Kernel::FT si = sin(angle);

	return Transformation	(1+co*(x*x-1),	-z*si+co*x*y,	y*si+co*x*z,		// Rodrigues formula for the rotation matrix
							 z*si+co*x*y,	1+co*(y*y-1),	-x*si+co*y*z,
							 -y*si+co*x*z,	x*si+co*y*z,	1+co*(z*z-1));
}

std::vector<Nef_polyhedron> splitNefs (Nef_polyhedron N, bool doCut, bool onlyExteriorShells) {
	N.regularization();

	bool pray=false;
	if (MANIFOLD_FIRST && (pray=!make2Manifold(N,-1, doCut)))				// REMOVE THIS IF "Nef_polyhedron(N,si)" WORKS on non-manifolds!
		std::cerr << "WARNING: splitting non-manifold.., pray that it works, might cause infinite loop. Try splitFirst= false if it fails"<< std::endl;

	std::vector<Nef_polyhedron> nefVector;
	if (N.number_of_volumes()>2){
		Nef_polyhedron::Volume_const_iterator vi = ++N.volumes_begin();		// Init iterator, skip COMPLETE start at 1st=outershell
		CGAL_forall_volumes(vi,N) {											// Loop over all volumes except COMPLETE
			if(vi->mark()) {												// Check whether vi is part of ...?
				bool first = true;
				Nef_polyhedron::Shell_entry_const_iterator si;				// SFaces
				CGAL_forall_shells_of(si, vi) {		
					if (first || !onlyExteriorShells) {
						nefVector.push_back(Nef_polyhedron(N,si));//sfch));	// Store shell as nef in vector
						first = false;
					} else
						break;												// Possible ROOMS <<----
				}
			}
		}
	} else nefVector.push_back(N);
	if (pray)	std::cerr << "..Phew.."<<std::endl;
	return nefVector;
}

#ifdef WIN32 // else replace with nary union if possible
// nefPolyheVector should at least contain 1 nef else returns Nef_polyhedron::EMPTY
Nef_polyhedron mergeNefs (std::vector<Nef_polyhedron>& nefPolyheVector, int iteration)  {
	if (nefPolyheVector.size()!=0) {
		if(nefPolyheVector.size()!=1 && iteration < 1+__lzcnt(nefPolyheVector.size()-1)) {  // < nbrOfItr // lzcnt seems to be bits-#lz
			int nefCount = 0;
			int bad = (2<<iteration), good = (2<<(iteration-1)), idxthing = ~(bad-1);
			for (std::vector<Nef_polyhedron>::iterator nIt=nefPolyheVector.begin();nIt!=nefPolyheVector.end();nIt++,nefCount++)
				if (nefCount%bad!=0 && (iteration==0||nefCount%good==0))
					nefPolyheVector[nefCount & idxthing] += *nIt;			//idx=2^(1+itr)		if nefcount%idx!=0					// Combine two nefs time is spend here
			return mergeNefs (nefPolyheVector, ++iteration);
		}
		return nefPolyheVector[0];								// Normal return statement
	}
	else return Nef_polyhedron(Nef_polyhedron::EMPTY);
}
#else
// Should work, not tested
// nefPolyheVector should at least contain 1 nef else returns Nef_polyhedron::EMPTY
Nef_polyhedron mergeNefs (std::vector<Nef_polyhedron> nefPolyheVector)  {
	if(nefPolyheVector.size() != 1) {
		std::vector<Nef_polyhedron> shorterNefPolyheVector((nefPolyheVector.size()+1)>>1);	// Create vector half the size (rounded up) of the current
		int nefCount=0;
		for (std::vector<Nef_polyhedron>::reverse_iterator nIt=nefPolyheVector.rbegin();nIt!=nefPolyheVector.rend();nIt++,nefCount++)
			shorterNefPolyheVector[nefCount>>1] += *nIt;
		return mergeNefs (shorterNefPolyheVector);
	}
	else return Nef_polyhedron(Nef_polyhedron::EMPTY);
	return nefPolyheVector[0];
}
#endif

// Replaces default nef constructor which doesn't always work
// Regularization is optional
// Returns true if succesfull
bool polyhe2nef (Polyhedron& polyhe, Nef_polyhedron& nefPolyhe, bool regular, bool normal, bool useDefault) {
	bool succesBool = false;

	if (normal) polyhe.normalize_border();			// Makes it mo betta'  ?
	if (polyhe.is_closed()) {						// Check if simple
		if (polyhe.is_valid()) {					// Check if valid
			succesBool = true;
			if (!polyhe.normalized_border_is_valid()) std::cerr << "WARNING: Polyhedron border is not normalized (polyhe2nef)" << std::endl;
		} else std::cerr << "WARNING: Polyhedron is not valid (polyhe2nef)" << std::endl;
	} else std::cerr << "WARNING: Polyhedron is not closed (polyhe2nef)" << std::endl;
	if (!succesBool) return false;

	if (useDefault) nefPolyhe = Nef_polyhedron(polyhe);
	else {
		std::stringstream ssPolyhe;					// Init stringstream
		ssPolyhe.precision(SSTREAM_PRECISION);		// Set precision
		ssPolyhe << polyhe;							// Write polyhe to stringstream
		CGAL::OFF_to_nef_3(ssPolyhe, nefPolyhe);	// Create nef from stringstream
	}
	if (regular) nefPolyhe.regularization();		// Regularize nef 

	if (nefPolyhe.is_simple()) {					// Check if simple
		if (nefPolyhe.is_valid()) {					// Check is valid
			return succesBool;
		} else std::cerr << "WARNING: Nef is not valid (polyhe2nef)" << std::endl;
	} else std::cerr << "WARNING: Nef is not simple (polyhe2nef)" << std::endl;
	return false;
}

// Regularization is optional
// Returns true if succesfull
bool nef2polyhe (Nef_polyhedron& nefPolyhe, Polyhedron& polyhe, bool regular, bool normal) {
	bool succesBool = false;

	if (regular) nefPolyhe.regularization();	// Regularize nef 
	if (nefPolyhe.is_simple()) {				// Check if simple
		if (nefPolyhe.is_valid()) {				// Check is valid
			succesBool = true;
		} else std::cerr << "WARNING: Nef is not valid (nef2polyhe)" << std::endl;
	} else std::cerr << "WARNING: Nef is not simple (nef2polyhe)" << std::endl;
	if (!succesBool) return false;

	nefPolyhe.convert_to_polyhedron(polyhe);
	
	if (normal) polyhe.normalize_border();	// Mo mo betta' ! ?
	if (polyhe.is_closed()) {				// Check if simple
		if (polyhe.is_valid()) {			// Check is valid
			if (!polyhe.normalized_border_is_valid()) std::cerr << "WARNING: Polyhedron border is not normalized (polyhe2nef)" << std::endl;
			return succesBool;
		} else std::cerr << "WARNING: Polyhedron is not valid (polyhe2nef)" << std::endl;
	} else std::cerr << "WARNING: Polyhedron is not closed (polyhe2nef)" << std::endl;
	return false;
}


Nef_polyhedron createBBox (Nef_polyhedron& nefPolyhe) {
	Polyhedron polyhe, boundingBox;
	
	nef2polyhe(nefPolyhe,polyhe);
	Polyhedron::Vertex_const_iterator vit;
	Point_3 initPoint = polyhe.vertices_begin()->point();
	double xmin = CGAL::to_double(initPoint.x()), xmax = CGAL::to_double(initPoint.x()),
		ymin = CGAL::to_double(initPoint.y()), ymax = CGAL::to_double(initPoint.y()),
		zmin = CGAL::to_double(initPoint.z()), zmax = CGAL::to_double(initPoint.z());

	for (vit=polyhe.vertices_begin();vit!=polyhe.vertices_end();++vit){
		Point_3 vertex = vit->point();
		double x = CGAL::to_double(vertex.x()),
			y = CGAL::to_double(vertex.y()),
			z = CGAL::to_double(vertex.z());
		if (x > xmax) xmax = x;
		if (x < xmin) xmin = x;
		if (y > ymax) ymax = y;
		if (y < ymin) ymin = y;
		if (z > zmax) zmax = z;
		if (z < zmin) zmin = z;
	}
	xmin-=5; xmax+=5; ymin-=5; ymax+=5; zmin-=5; zmax+=5;
	Point_3 p(xmin,ymin,zmin), q(xmax,ymax,zmax);
	Bounding_box bbox(p,q);
	pointVector convexPoints;
	for (int i = 0; i<8; ++ i) {
		convexPoints.push_back(bbox.vertex(i));
	}
	CGAL::convex_hull_3(convexPoints.begin(),convexPoints.end(),boundingBox);
	return Nef_polyhedron (boundingBox);
}

Polyhedron createBBox (Polyhedron& polyhe) {
	Polyhedron boundingBox;
	
	Polyhedron::Vertex_const_iterator vit;
	Point_3 initPoint = polyhe.vertices_begin()->point();
	double xmin = CGAL::to_double(initPoint.x()), xmax = CGAL::to_double(initPoint.x()),
		ymin = CGAL::to_double(initPoint.y()), ymax = CGAL::to_double(initPoint.y()),
		zmin = CGAL::to_double(initPoint.z()), zmax = CGAL::to_double(initPoint.z());

	for (vit=polyhe.vertices_begin();vit!=polyhe.vertices_end();++vit){
		Point_3 vertex = vit->point();
		double x = CGAL::to_double(vertex.x()),
			y = CGAL::to_double(vertex.y()),
			z = CGAL::to_double(vertex.z());
		if (x > xmax) xmax = x;
		if (x < xmin) xmin = x;
		if (y > ymax) ymax = y;
		if (y < ymin) ymin = y;
		if (z > zmax) zmax = z;
		if (z < zmin) zmin = z;
	}
	Point_3 p(xmin,ymin,zmin), q(xmax,ymax,zmax);
	Bounding_box bbox(p,q);
	pointVector convexPoints;
	for (int i = 0; i<8; ++ i) {
		convexPoints.push_back(bbox.vertex(i));
	}
	CGAL::convex_hull_3(convexPoints.begin(),convexPoints.end(),boundingBox);
	return boundingBox;
}