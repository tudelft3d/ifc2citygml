#include "ManifoldFix.h"

void add_neighbourhood_to_hullPoints(pointVector& hullPoints, const Vertex_const_handle& vert, const double& tapeSize) {
	Point_3 pnt = vert->point();
	hullPoints.push_back(pnt);								// Add vertex point
	Nef_polyhedron::SVertex_const_iterator svcIt = vert->svertices_begin(), svcItEND = vert->svertices_end();
	CGAL_For_all(svcIt,svcItEND) {
		Vector_3 vecR(pnt,svcIt->target()->point());
		Vector_3 vecRnew = vecR * tapeSize / std::sqrt(CGAL::to_double(vecR.squared_length()));
		if ((vecR.squared_length()-OVERLAP_DIST_THRESHOLD) > vecRnew.squared_length())
			hullPoints.push_back(pnt+vecRnew);						// Add svertex neighbourhood point (tapesize away from vertex)
		else
			hullPoints.push_back(svcIt->target()->point());
	}
}

#ifndef MANIFOLD_CUBE_FIX

bool make2Manifold(Nef_polyhedron& nefPolyhe, double fixSize, bool doCut) {

	nefPolyhe.regularization();

	int		maxItr		= MANIFIX_MAX_ITR;
	double	tapeSize	= FIX_MANIFOLD_SIZE;
	if (fixSize>0) tapeSize = fixSize;

	Nef_polyhedron::Vertex_const_iterator vertIt;
	Nef_polyhedron::Halfedge_const_iterator edgeIt;

	int itr;	
	unsigned int numberManis = 0;
	for (itr=0;itr<maxItr;itr++) {
				
		NefNaryUnion naryEdgeNef;
		bool foundNM = false;
		int edgeCounter = 0;
		CGAL_forall_edges(edgeIt,nefPolyhe) {	// Loop all edges
			++edgeCounter;
			if(!is_edge_2manifold(edgeIt)) {	// Check if nonmanifold (Should find all non-manif-edges first then fix all and recheck instead)
				++numberManis;	foundNM = true;
				std::cerr << "\rWARNING: New non-manifold edge found,   total: "<<numberManis<<". ("<< 80*edgeCounter/nefPolyhe.number_of_edges()<<"%)  ";

				pointVector hullPoints;													// Get neighbourhood points
				add_neighbourhood_to_hullPoints(hullPoints,edgeIt->source(),tapeSize);
				add_neighbourhood_to_hullPoints(hullPoints,edgeIt->target(),tapeSize);
				
				Polyhedron polyHull;	
				CGAL::convex_hull_3(hullPoints.begin(),hullPoints.end(),polyHull);		// Create convexhull
				naryEdgeNef.add_polyhedron(Nef_polyhedron(polyHull));			
		}	}
		if (foundNM) {
			std::cerr <<"\rWARNING: Fixing Non-manifold edges      total: "<<numberManis<<".         ";
			if (doCut)	nefPolyhe -= naryEdgeNef.get_union();							// Cut from nef
			else		nefPolyhe += naryEdgeNef.get_union();							// Add to nef
			nefPolyhe.regularization();	
			continue;					// Restart when a non-manifold edge was found
		}

		NefNaryUnion naryVertNef;
		int vertCounter = 0;
		CGAL_forall_vertices(vertIt,nefPolyhe) {
			++vertCounter;
			if(!is_vertex_2manifold(vertIt)) {
				++numberManis;	foundNM = true;
				std::cerr << "\rWARNING: New non-manifold vertex found, total: "<<numberManis<<". ("<< 80+20*vertCounter/nefPolyhe.number_of_vertices()<<"%)  ";

				pointVector hullPoints;													// Get neighbourhood points
				add_neighbourhood_to_hullPoints(hullPoints,vertIt,tapeSize);
				
				Polyhedron polyHull;	
				CGAL::convex_hull_3(hullPoints.begin(),hullPoints.end(),polyHull);		// Create convexhull
				naryVertNef.add_polyhedron(Nef_polyhedron(polyHull));				
		}	}
		if (foundNM) {
			std::cerr <<"\rWARNING: Fixing Non-manifold vertices   total: "<<numberManis<<".         ";
			if (doCut)	nefPolyhe -= naryVertNef.get_union();							// Cut from nef
			else		nefPolyhe += naryVertNef.get_union();							// Add to nef
			nefPolyhe.regularization();	
			continue;					// Restart when a non-manifold edge was found
		}
		else break;						// Don't restart when all edges and vertices are 2manifold
	}
	if (numberManis==maxItr) {
		std::cerr << "\nERROR: More than " << numberManis << " non-manifold cases found. Fixing stopped" <<std::endl;
		return false;
	}
	if (itr>0) std::cerr << "\rWARNING:   Total number of non-manifold cases: "<<numberManis<<". (100%)  " <<std::endl;
	return true;
}

#else

bool make2Manifold(Nef_polyhedron& nefPolyhe, double fixSize, bool doCut) {

	nefPolyhe.regularization();

	int		maxItr		= MANIFIX_MAX_ITR;
	double	tapeSize	= FIX_MANIFOLD_SIZE;
	if (fixSize>0) tapeSize = fixSize;

	Nef_polyhedron::Vertex_const_iterator vertIt;
	Nef_polyhedron::Halfedge_const_iterator edgeIt;

	Nef_polyhedron boxNEF;													// Get box
	get_unitBox	(boxNEF);
	//if (commonDir != CGAL::NULL_VECTOR)										// Rotate box
	//	boxNEF.transform(getRotationMatrix(Vector_3(1,0,0),commonDir));		// Set Vector_3(1,1,0) to rotate to 45 deg
	Transformation scale(CGAL::SCALING, tapeSize);							// Scale box
	boxNEF.transform(scale);
	int itr;	
	unsigned int numberManis = 0;
	for (itr=0;itr<maxItr;itr++) {
		bool foundNM = false;
		int edgeCounter = 0;
		CGAL_forall_edges(edgeIt,nefPolyhe) {	// Loop all edges
			++edgeCounter;
			if(!is_edge_2manifold(edgeIt)) {	// Check if nonmanifold
				++numberManis;
				std::cerr << "\rWARNING: New non-manifold edge found,   total: "<<numberManis<<". ("<< 80*edgeCounter/nefPolyhe.number_of_edges()<<"%) ";
				
				Nef_polyhedron uNef = boxNEF;
				Point_3 p1 = edgeIt->source()->point();									// Get end points
				Point_3 p2 = edgeIt->target()->point();

				pointVector fixPoints;
				Nef_polyhedron uNef2 = boxNEF;
				
				Transformation toP1(CGAL::TRANSLATION, Vector_3(CGAL::ORIGIN,p1));		// Get translation vector to p1
				uNef2.transform(toP1);													// Apply transformations
				CGAL_forall_vertices(vertIt,uNef2)
					fixPoints.push_back(vertIt->point());								// Add points to convexhull
				Transformation toP2(CGAL::TRANSLATION, Vector_3(p1,p2));				// Get translation to p2 and apply
				uNef2.transform(toP2);
				CGAL_forall_vertices(vertIt,uNef2)
					fixPoints.push_back(vertIt->point());								// Add points to convexhull


				Polyhedron polyHull;	
				CGAL::convex_hull_3(fixPoints.begin(),fixPoints.end(),polyHull);		// Create convexhull
				Nef_polyhedron nefHull(polyHull);	

				if (doCut)	nefPolyhe -= nefHull;										// Cut from nef
				else		nefPolyhe += nefHull;										// Add to nef
				nefPolyhe.regularization();												// regularize
				foundNM = true;
				break;
			}
		}
		if (foundNM) continue;

		int vertCounter = 0;
		CGAL_forall_vertices(vertIt,nefPolyhe) {
			++vertCounter;
			if(!is_vertex_2manifold(vertIt)) {
				++numberManis;
				std::cerr << "\rWARNING: New non-manifold vertex found, total: "<<numberManis<<". ("<< 80+20*vertCounter/nefPolyhe.number_of_vertices()<<"%)";
				Nef_polyhedron uNef = boxNEF;
				Transformation toCentroid(CGAL::TRANSLATION, Vector_3(CGAL::ORIGIN,vertIt->point())); 
				uNef.transform(toCentroid);
				if (doCut)	nefPolyhe -= uNef;
				else		nefPolyhe += uNef;
				nefPolyhe.regularization();
				foundNM = true;
				break;
			}
		}
		if (!foundNM) break;					// Stop if all edges are 2manifold
	}
	if (numberManis==maxItr) {
		std::cerr << "\nERROR: More than " << numberManis << " non-manifold cases found. Fixing stopped" <<std::endl;
		return false;
	}
	if (itr>0) std::cerr << "\rWARNING:   Total number of non-manifold cases: "<<numberManis<<". (100%)" <<std::endl;
	return true;
}

#endif

bool is_edge_2manifold(const Halfedge_const_handle& e) {
	SM_decorator SD;
	Nef_polyhedron::SHalfedge_around_svertex_const_circulator c(SD.first_out_edge(e)), c2(c);
	if(c == 0)	  return false;
	if(++c == c2) return false;
	if(++c != c2) return false;
	return true;
}

bool is_vertex_2manifold(const Vertex_const_handle& v) {     
    SFace_const_iterator sfi(v->sfaces_begin());
    return ++sfi == v->sfaces_last();
}

std::pair<Vector_3,Vector_3> getMostCommonVector(Polyhedron& polyhe, bool NOzAxis) {  // not really tested but seems to work
	if (!polyhe.is_pure_triangle()) std::cerr << "ERROR: Not pure Triangle.";
	std::transform( polyhe.facets_begin(), polyhe.facets_end(), polyhe.planes_begin(), Plane_equation()); 

	double tol = 5e-2;														// Coplanar tolerance

	std::vector<std::vector<std::pair<Vector_3,double>>>	fNormals;		// facenormals and their area in coplanar vectors
	std::vector<std::pair<Vector_3,double>>					avgFN;


	Polyhedron::Facet_const_iterator fcIt,end;
	fcIt = polyhe.facets_begin();
	end =  polyhe.facets_end();
	CGAL_For_all(fcIt,end) {												// Loop over all faces
		if (fcIt->plane().is_degenerate()) {
			std::cerr << "ERROR: Facet is degenerate." <<std::endl;
			continue;
		}
		
		double	 area		= sqrt(CGAL::to_double(comp_facetSquaredArea(fcIt)));// Get area
		Vector_3 faceNormal	= fcIt->plane().orthogonal_vector();			// Get normal vector

		if (NOzAxis) faceNormal = Vector_3(faceNormal.x(),faceNormal.y(),0);// Set Z to zero
		if (faceNormal.squared_length()==0) continue;

		Vector_3 unitFaceNormal = faceNormal / sqrt(CGAL::to_double(faceNormal.squared_length()));
		std::pair<Vector_3,double> VApair(unitFaceNormal,area);

		bool coplanar = false;
		for (int i=0; i<(int)fNormals.size(); i++) {										// Iterate over stored facenormals

			Kernel::FT interFaceSP = unitFaceNormal * (avgFN[i].first);				// Compare to average only!!! Average might diverge from origin

			if (CGAL::compare (interFaceSP, 1+tol) == CGAL::SMALLER && 
				CGAL::compare (interFaceSP, 1-tol) == CGAL::LARGER) {				// If Coplanar with any of the previous faces
				coplanar = true;
				fNormals[i].push_back(VApair);										// Add Vector_3 to list

				Vector_3 newAverageVector = CGAL::NULL_VECTOR;						// Calcuate new weighted average
				double totalArea=0;													// Calculate total contributing area
				for (int j=0; j<(int)fNormals[i].size(); j++) {
					newAverageVector = newAverageVector + fNormals[i][j].first * fNormals[i][j].second;
					totalArea += fNormals[i][j].second;
				}
				avgFN[i] = std::pair<Vector_3,double> (newAverageVector / sqrt(CGAL::to_double(newAverageVector.squared_length())), totalArea);
				break;
			}
			Kernel::FT negInterFaceSP = (-unitFaceNormal) * (avgFN[i].first);		// Compare to REVERSED 1st/average only!!! Average might diverge from origin
			if (CGAL::compare (negInterFaceSP, 1+tol) == CGAL::SMALLER && 
				CGAL::compare (negInterFaceSP, 1-tol) == CGAL::LARGER) {
				coplanar = true;
				VApair.first = -unitFaceNormal;										// REVERSE VECTOR
				fNormals[i].push_back(VApair);										// Add Vector_3 to list

				Vector_3 newAverageVector = CGAL::NULL_VECTOR;						// Calcuate new weighted average
				double totalArea=0;													// Calculate total contributing area
				for (int j=0; j<(int)fNormals[i].size(); j++) {
					newAverageVector = newAverageVector + fNormals[i][j].first * fNormals[i][j].second;
					totalArea += fNormals[i][j].second;
				}				
				avgFN[i] = std::pair<Vector_3,double> (newAverageVector / sqrt(CGAL::to_double(newAverageVector.squared_length())), totalArea);
				break;
			}
		}
		if (!coplanar) {															// If not coplanar with any of the previous faces
			avgFN.push_back(VApair);												// add new and set asa average
			fNormals.push_back(std::vector<std::pair<Vector_3,double>>(1,VApair));
		}
	}

	int biggest = avgFN[0].second < avgFN[1].second;	
	double maxArea1				= avgFN[ biggest].second;
	double maxArea2				= avgFN[!biggest].second;
	Vector_3 mostCommonVector1	= avgFN[ biggest].first;
	Vector_3 mostCommonVector2	= avgFN[!biggest].first;						

	for (int i=2; i<(int)avgFN.size(); i++) {											// Get vector with the largest area
		if ( avgFN[i].second > maxArea2){
			if ( avgFN[i].second > maxArea1){
				mostCommonVector2	= mostCommonVector1;
				maxArea2			= maxArea1;
				mostCommonVector1	= avgFN[i].first;
				maxArea1			= avgFN[i].second;
			} else {
				mostCommonVector2	= avgFN[i].first;
				maxArea2			= avgFN[i].second;
			}
		}
	}
	Vector_3 perpCommonVec2 = CGAL::cross_product(mostCommonVector1,mostCommonVector2); // Make sure the second vector is perpendicular to the first (assumes a cube)
	perpCommonVec2 = perpCommonVec2 / sqrt(CGAL::to_double(perpCommonVec2.squared_length()));

	return std::pair<Vector_3,Vector_3>(mostCommonVector1,perpCommonVec2);
}

//void (fix_degenerate(Polyhedron& polyhe)) {
//	Kernel::FT lengthThreshold = MIN_LENGTH_THRESHOLD;
//	unsigned int itr = 0;
//	unsigned int degens = 0;
//	while (true) {
//		++itr;
//		bool degenFound = false;
//		int edgeCounter = 1;
//		for (Polyhedron::Halfedge_iterator hIt=polyhe.halfedges_begin();hIt!=polyhe.halfedges_end();++hIt,++edgeCounter) {
//			bool succes = false;
//			if (CGAL::squared_distance(hIt->vertex()->point(),hIt->opposite()->vertex()->point()) < lengthThreshold)
//				succes = do_edgeCollapse(polyhe,hIt);
//			if (succes) {
//				++degens;
//				std::cerr << "\rWARNING: "<<degens<< " degenerate triangle(s) (fix_degenerate: "<< 100*edgeCounter/polyhe.size_of_halfedges()<<"%)";
//				degenFound = true;
//				break;
//			}
//		}
//		if (!degenFound) break;
//	}
//	if (degens > 0)
//		std::cout << "\rWARNING: "<<degens<< " degenerate triangle(s) (fix_degenerate: 100%)" <<std::endl;
//	else
//		std::cout << "\r         "<<degens<< " degenerate triangle(s) (fix_degenerate: 100%)" <<std::endl;
//}

void fix_degenerate(Polyhedron& polyhe) {
	if (polyhe.is_pure_triangle()) {

		Kernel::FT lengthThreshold = MIN_LENGTH_THRESHOLD;
		int		maxItr		= MANIFIX_MAX_ITR;
		int		itr			= 0;
		while (true) {
			++itr;
			bool degenFound = false;
			int facetCounter = 1;
			for (Polyhedron::Facet_iterator fit = polyhe.facets_begin(); fit != polyhe.facets_end(); ++fit,++facetCounter) {	// Iterate over faces

				pointVector facePoints;
				Polyhedron::Facet::Halfedge_around_facet_circulator itHDS = fit->facet_begin();
				do facePoints.push_back(itHDS->vertex()->point());												// Get vertexes of face
				while (++itHDS != fit->facet_begin());

				if (triangleIsDegenerate(facePoints)) {								// Check if degenerate
					std::cerr << "\rWARNING: "<<itr<< " degenerate triangle(s) (fix_degenerate: "<< 100*facetCounter/polyhe.size_of_facets()<<"%)";
					
					SortDegPairs degPairs = sort_degenTri(fit);						// Get shortest length, shortestHE and longestHE

					bool succes = false;
					if (degPairs.first>=lengthThreshold) {							// If shortest edge is longer than threshold -> do_removeCap
						//std::cerr << std::endl<< "WARNING: Cap" << std::endl;
						//succes = do_removeCap(polyhe, degPairs.second.second,lengthThreshold);	// NOT SAFE ENOUGH
						succes = do_edgeFlip(polyhe, degPairs.second.second->opposite());
						succes = true;
					}
					else if (!succes)
						succes = do_edgeCollapse(polyhe,degPairs.second.first);		// Try to collapse the shortest edge
					if (!succes) {
						//std::cerr << std::endl<< "WARNING: Could not fix degenerate Triangle" << std::endl;
						continue;
					} else {
						degenFound = true;
						break;
					}
				}
			}
			if (!degenFound)break;
		}
		//if (itr>=maxItr) std::cerr <<std::endl<< "ERROR: More than " << maxItr << " degenerate cases found. Fixing stopped" <<std::endl;
		//if (itr>1)
		if (itr>1) std::cout << "\rWARNING: "<<itr<< " degenerate triangle(s) (fix_degenerate: 100%)" <<std::endl;
	} else std::cerr << "ERROR: Not pure triangle (fix_degenerate)"<< std::endl;
	
}

SortDegPairs sort_degenTri(Polyhedron::Facet_handle& fch) {
	Polyhedron::Facet::Halfedge_around_facet_circulator itHDS = fch->facet_begin();

	Kernel::FT length1 = MAX_VALUE;
	Kernel::FT length2 = MAX_VALUE;
	Polyhedron::Halfedge_handle shortHE;
	Polyhedron::Halfedge_handle mediumHE;
	Polyhedron::Halfedge_handle longHE;
	for (int i=0;i<3;i++,itHDS++){										// Get distance of each edge
		Kernel::FT tempLen = CGAL::squared_distance(itHDS->prev()->vertex()->point(),itHDS->vertex()->point());
		if (tempLen < length2) {										// Sort edges on length (could use sort function..)
			if (tempLen < length1) {
				length2  = length1;
				length1  = tempLen;
				longHE   = mediumHE;
				mediumHE = shortHE;
				shortHE  = itHDS;					
			} else {
				length2  = tempLen;
				longHE   = mediumHE;
				mediumHE = itHDS;
			}
		} else  longHE   = itHDS;
	}
	return SortDegPairs(length1,HalfedgePair(shortHE,longHE));
}


bool triangleIsDegenerate(pointVector& triPoints) {
	Triangle_3 tri(triPoints[0],triPoints[1],triPoints[2]);
	if (tri.is_degenerate())
		return true;
	//for (int i=0;i<3;i++){										
	//	Vector_3 cur(triPoints[i],triPoints[(i+1)%3]);
	//	Vector_3 nex(triPoints[(i+1)%3],triPoints[(i+2)%3]);
	//	if (is_colinear(cur,nex) || is_colinear(cur,-nex))
	//		return true;
	//}
	return false;
}

bool triangleIsDegenerate(Polyhedron::Facet_handle& fch) {
	return triangleIsDegenerate(comp_facetPoints(fch));
}

// Attempt to remove cap. Fails if opposite face creates new caps due to split
bool do_removeCap(Polyhedron& polyhe,Polyhedron::Halfedge_handle longHE,const Kernel::FT& lengthThreshold) {

	Segment_3 seg(longHE->prev()->vertex()->point(),longHE->vertex()->point());
	Line_3 ln(seg);
	Point_3 projPnt = ln.projection(longHE->next()->vertex()->point());
	if (!seg.has_on(projPnt)) std::cerr <<std::endl<< "WARNING: Projected point not on segment (do_removeCap)" <<std::endl;

	Polyhedron::Halfedge_handle newEdge = polyhe.split_edge(longHE);														// Add vertex on the longest edge
	// Do pre-check here!
	newEdge->vertex()->point() = projPnt;																					// Move inserted point to projected location
	Polyhedron::Halfedge_handle otherFaceEdge = polyhe.split_facet(longHE->opposite(),longHE->opposite()->next()->next());	// Split neighbouring face

	if ((triangleIsDegenerate(otherFaceEdge->facet())				&& sort_degenTri(otherFaceEdge->facet()).first				>=lengthThreshold) ||
		(triangleIsDegenerate(otherFaceEdge->opposite()->facet())	&& sort_degenTri(otherFaceEdge->opposite()->facet()).first	>=lengthThreshold)) {
		std::cerr <<std::endl<< "WARNING: Cap fix would cause new cap (do_removeCap)" <<std::endl;
		polyhe.join_facet(otherFaceEdge);																					// Restore original state
		polyhe.join_vertex(newEdge);
		return false;
	} else {
		Polyhedron::Halfedge_handle colapMe = polyhe.split_facet(newEdge,longHE->next());									// Split face at projected point
		// Should check whether any of the other moved triangles hasn't become a cap otherwise it is not safe
		return do_edgeCollapse(polyhe, colapMe);																			// Collapse the newly created edge
	}
}

// Only use when edge is short
bool do_edgeCollapse(Polyhedron& polyhe, Polyhedron::Halfedge_handle h) {
	if (CGAL::circulator_size(h->vertex_begin()) == 3) {										
		polyhe.erase_center_vertex(h);															// Probably crashes near border edges
	} else if(CGAL::circulator_size(h->vertex_begin()) > 3			&&							// Should not fail if closed
			  CGAL::circulator_size(h->next()->vertex_begin())>=3	&&
			  CGAL::circulator_size(h->opposite()->prev()->vertex_begin())>=3 ) {
				  polyhe.join_facet(h->next()->opposite());										// Remove left edge
				  polyhe.join_facet(h->opposite()->prev()->opposite());							// Remove right edge
				  polyhe.join_vertex(h);//->facet()->semanticBLA = "asasd";						// Collapse edge
	} else {
		std::cerr<< std::endl<<"WARNING: Could not collapse edge."<<std::endl;
		return false;
	}
	return true;
}

// Use only if coplanar and same semantics better then removecap thing, nah would be remove by de-triangulation
// Removes face of h->opposite()
bool do_edgeFlip(Polyhedron& polyhe, Polyhedron::Halfedge_handle h, bool checkDegen) {

	if (CGAL::circulator_size(h->vertex_begin())>=3&&									// Pre-checks
		CGAL::circulator_size(h->opposite()->vertex_begin())>=3 &&
		CGAL::circulator_size( h->facet_begin())== 3&&
		CGAL::circulator_size( h->opposite()->facet_begin())== 3) {
			Polyhedron::Facet::Halfedge_handle heC = polyhe.join_facet(h);				// Edge flip
			heC = polyhe.split_facet(heC->prev(),heC->next());

			if (checkDegen &&
				(triangleIsDegenerate(comp_facetPoints(heC->facet())) ||				// Edge flip did not fix it, still degenerate
				 triangleIsDegenerate(comp_facetPoints(heC->opposite()->facet())))) {
					heC = polyhe.join_facet(h);
					heC = polyhe.split_facet(heC->prev(),heC->next());
					return false;
			}
	} else {
		std::cerr<< std::endl<<"WARNING: Could not flip edge."<<std::endl;
		return false;
	}
	return true;
}

void round_Vertex_3(Polyhedron::Vertex_handle& vrt) {
	vrt->point() = round_Point_3(vrt->point());
}

Point_3 round_Point_3(Point_3& pnt) {
	std::stringstream s;
	double x,y,z;
	s.str(std::string());s.clear();	s << std::setprecision(OUTPUT_DECIMALS) << std::setiosflags(std::ios_base::fixed) << CGAL::to_double(pnt.x());	s >> x;
	s.str(std::string());s.clear();	s << std::setprecision(OUTPUT_DECIMALS) << std::setiosflags(std::ios_base::fixed) << CGAL::to_double(pnt.y());	s >> y;
	s.str(std::string());s.clear();	s << std::setprecision(OUTPUT_DECIMALS) << std::setiosflags(std::ios_base::fixed) << CGAL::to_double(pnt.z());	s >> z;
	return Point_3(x,y,z);
}

bool roundCoordsSafely (Polyhedron& exteriorPolyhe,std::vector<Nef_polyhedron>& nefVec) {
	Polyhedron tempPolyhe(exteriorPolyhe);
	Nef_polyhedron exteriorNef,nefPolyhe;
	int itr=0;

	while(true) {			
		for (Polyhedron::Vertex_iterator vIt=tempPolyhe.vertices_begin();vIt!=tempPolyhe.vertices_end();++vIt)
			round_Vertex_3(vIt);

		nefPolyhe = Nef_polyhedron::EMPTY; /// <<?!?!?!?works r not
		if (!polyhe2nef(tempPolyhe,nefPolyhe))// std::cerr << "ERROR: Should not happen (roundCoordsSafely)"<<std::endl; // Prob just not simple
			tempPolyhe.clear();
		if (nef2polyhe(nefPolyhe,tempPolyhe))
			break;

		nefPolyhe.regularization();
		//nefPolyhe = splitNefs(nefPolyhe)[0];		// should be here 
		if (!make2Manifold(nefPolyhe,FIX_MANIFOLD_SIZE*(1<<itr))) return false;
		nefPolyhe = splitNefs(nefPolyhe)[0];		// not here

		nef2polyhe(nefPolyhe,tempPolyhe);


	//	//nefPolyhe.clear();
	//	//nefPolyhe = Nef_polyhedron.element_type`;
		//Nef_polyhedron 
		//if (!polyhe2nef(tempPolyhe,nefPolyhe))// std::cerr << "ERROR: Should not happen (roundCoordsSafely)"<<std::endl; // Prob just not simple
		//	tempPolyhe.clear();	//// somethinnnsss wrroonnggg right around herer?!?!?
		//if (nef2polyhe(nefPolyhe,tempPolyhe))
		//	break;

		//if (itr==1) exteriorNef = nefPolyhe;

		//nefPolyhe = makeExteriorManifold(exteriorNef,nefVec,FIX_MANIFOLD_SIZE*(1<<itr));

		//nef2polyhe(nefPolyhe,tempPolyhe);
		//++itr;
	} 
	//exteriorPolyhe.clear();
	exteriorPolyhe = tempPolyhe;
	return true;
}