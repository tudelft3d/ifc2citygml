#include "Minkowski.h"

Nef_polyhedron makeExteriorManifold(Nef_polyhedron extManiNef, std::vector<Nef_polyhedron>& nefVec,double fixSize, bool doCut) {
	extManiNef = separateFirstNef(extManiNef, nefVec);
	make2Manifold(extManiNef,fixSize);
	extManiNef = separateFirstNef(extManiNef, nefVec);
	return extManiNef;
}

Nef_polyhedron separateFirstNef(Nef_polyhedron& nefPolyhe, std::vector<Nef_polyhedron>& nefVec) {
	nefPolyhe.regularization();
	std::vector<Nef_polyhedron> tempNV = splitNefs (nefPolyhe);			// Extract largest Nef/exterior // Problems <<----
	Nef_polyhedron firstNefPolyhe = tempNV[0];
	nefVec.reserve( nefVec.size() + tempNV.size() - 1);					// preallocate memory bs
	nefVec.insert( nefVec.end(), (tempNV.begin()+1), tempNV.end() );	// Combine bp vectors without the exterior
	return firstNefPolyhe;
}
Nef_polyhedron removeInnerShells(Nef_polyhedron& nefPolyhe) {
	
	return mergeNefs(splitNefs(nefPolyhe));			// Extract largest Nef/exterior // Problems <<----
}

void shrinkMinkowski(Nef_polyhedron& nefPolyhe, Nef_polyhedron& robot) {
	Nef_polyhedron complement = createBBox(nefPolyhe) - nefPolyhe;
	complement.regularization();
	nefPolyhe = nefPolyhe - CGAL::minkowski_sum_3(complement,robot);	// Should use complementcopy - CGAL::minkowski_sum_3(complement,robot);
	nefPolyhe.regularization();
}

bool closing_Minkowski(Nef_polyhedron& fusedNefPolyhe, Polyhedron& exteriorPolyhe, double offsetMagnitude, std::vector<Nef_polyhedron>& bpVector, bool useFacetMink) {
	
	fusedNefPolyhe.regularization();
	//if (!MANIFOLD_FIRST)
		fusedNefPolyhe = mergeNefs(splitNefs(fusedNefPolyhe,false));				// Remove inner shells

	Nef_polyhedron robot = makeRobot(offsetMagnitude);

	if (!useFacetMink) {
		std::cout << "Minkowski sum dilation... (Use minkOfFacets if fails)"<< std::endl;		// Expand
		fusedNefPolyhe =  CGAL::minkowski_sum_3(fusedNefPolyhe,robot);
	} else fusedNefPolyhe = minkowskiSum_Of_Facets(fusedNefPolyhe,robot);

	std::vector<Nef_polyhedron> bigBPVec;
	fusedNefPolyhe = separateFirstNef(fusedNefPolyhe,bigBPVec);					// Extract exterior  process the others shrink n everything!! (as bp's)
	
	if (!useFacetMink) {
		std::cout << "Minkowski sum erosion...  (Use minkOfFacets if fails)"<< std::endl;		// Shrink
		shrinkMinkowski(fusedNefPolyhe,robot);
	} else fusedNefPolyhe = minkowskiSum_Of_Facets(fusedNefPolyhe,robot,true);

	std::cout << "Making 2Manifold..."<< std::endl;									// Make Manifold
	fusedNefPolyhe = makeExteriorManifold(fusedNefPolyhe,bpVector);

	if (bigBPVec.size()>0) {
		Nef_polyhedron bigBP = mergeNefs(bigBPVec);									// not using nary as it's already in a vector 
		if (!bigBP.is_empty()) {
			std::cout << "Eroding and Splitting BuildingParts..."<< std::endl;		// Shrink
			shrinkMinkowski(bigBP,robot);	
			std::vector<Nef_polyhedron> tempNV = splitNefs (bigBP);					// Extract largest Nef/exterior // Problems <<----
			bpVector.reserve( bpVector.size() + tempNV.size());						// preallocate memory bs
			bpVector.insert( bpVector.end(), (tempNV.begin()), tempNV.end() );		// Combine bp vectors without the exterior
		}
	}
	return nef2polyhe(fusedNefPolyhe,exteriorPolyhe); // etc
}

// Doesn't crash, more memory safe(?) and SLOW !
Nef_polyhedron minkowskiSum_Of_Facets (Nef_polyhedron minkowskiNef, Nef_polyhedron& robot, bool subtract) {
	std::string		minkKindStr = "dilation ";		if (subtract)	minkKindStr = "shrinking";
	make2Manifold(minkowskiNef);
	Polyhedron polyhe;
	nef2polyhe(minkowskiNef,polyhe);				// Would be better on Nef (no-manifold needed), but difficult to get triangles
	int currFacet=0;
	for (Polyhedron::Facet_const_iterator fcIt = polyhe.facets_begin(); fcIt != polyhe.facets_end(); ++fcIt,++currFacet) {	// Iterate over faces	
		std::cout << "\rMinkowski Sum of Facets "<<minkKindStr<<": "<< currFacet<<"/"<<polyhe.size_of_facets()<<" - " << 100*currFacet/polyhe.size_of_facets()<<"%";
		
		pointVector facePoints = comp_facetPoints(fcIt);					// Get tri vertices
		//Nef_polyhedron face    (facePoints.begin(),facePoints.end());		// Create nef face from vertexes
		if (!subtract)	minkowskiNef += minkowskiSum_of_triangle(facePoints,robot);//CGAL::minkowski_sum_3(face,robot);	// Add to total minkowski nef
		else			minkowskiNef -= minkowskiSum_of_triangle(facePoints,robot);//CGAL::minkowski_sum_3(face,robot);	// Subtract to total minkowski nef.
	}
	minkowskiNef.regularization();
	std::cout << "\rMinkowski Sum of Facets "<<minkKindStr<<": "<< polyhe.size_of_facets()<<"/"<<polyhe.size_of_facets()<<" - 100%"<<std::endl;
	return minkowskiNef;
}

Nef_polyhedron minkowskiSum_of_triangle(pointVector& facePoints,Nef_polyhedron& robot) {
	pointVector hullPoints;
	Nef_polyhedron::Vertex_const_iterator vcIt;
	CGAL_forall_vertices(vcIt,robot) {
		for (pointVector::iterator pvIt=facePoints.begin();pvIt!=facePoints.end();++pvIt) {		// Could vectorize this..? nah not with kernel::FT
			hullPoints.push_back(vcIt->point() + (*pvIt-CGAL::ORIGIN));
		}
	}
	Polyhedron polyHull;	
	CGAL::convex_hull_3(hullPoints.begin(),hullPoints.end(),polyHull);		// Create convexhull
	return Nef_polyhedron(polyHull);	
}

Nef_polyhedron makeRobot (double offsetMagnitude, Nef_polyhedron& uNef, int shape, Vector_3 faceNormal, bool rotateRobot) {
	// Get unit Nef Geometry
	if(shape!=0) {
		std::ifstream robot_ifstream ("ClosingElement.off");
		if (robot_ifstream.is_open())
			CGAL::OFF_to_nef_3(robot_ifstream, uNef);
		else {
			std::cerr << "ERROR: Could not open ClosingElement.off, using default cube instead." <<std::endl;
			get_unitBox (uNef);
		}		
	}else get_unitBox (uNef); // Create unit cube nef-polyhe

	// Transform unit Nef
	Transformation scale(CGAL::SCALING, offsetMagnitude);
	uNef.transform(scale);															// Scale bot to size
	if (rotateRobot) uNef.transform(getRotationMatrix(Vector_3(0,0,1),faceNormal)); // Rotate to the plane of the face
	return uNef;
}

void get_unitBox(Nef_polyhedron& uNef) {
	std::stringstream uStream;
	uStream	<< "OFF"			<< std::endl
			<< "8 6 0"			<< std::endl
			<< "1 1 1"			<< std::endl
			<< "-1 1 1"			<< std::endl
			<< "1 -1 1"			<< std::endl
			<< "-1 -1 1"		<< std::endl
			<< "1 1 -1 "		<< std::endl
			<< "-1 1 -1"		<< std::endl
			<< "1 -1 -1"		<< std::endl
			<< "-1 -1 -1"		<< std::endl
			<< "4 1 3 2 0"		<< std::endl
			<< "4 2 6 4 0"		<< std::endl
			<< "4 4 5 1 0"		<< std::endl
			<< "4 5 4 6 7"		<< std::endl
			<< "4 6 2 3 7"		<< std::endl
			<< "4 3 1 5 7"		<< std::endl;
	CGAL::OFF_to_nef_3(uStream, uNef);
}