#include "SnapSemantics.h"

// Add semantics to Interior room polyhedra
void set_semantic_InteriorLoD4(Polyhedron& polyhe) {
	std::transform( polyhe.facets_begin(), polyhe.facets_end(),polyhe.planes_begin(),Plane_Newel_equation());		
	for (Polyhedron::Facet_iterator fIt = polyhe.facets_begin(); fIt != polyhe.facets_end(); ++fIt) {	// Iterate over faces
		Kernel::FT z = fIt->plane().orthogonal_vector().z();
		if		(z <= -HORIZONTAL_ANGLE_RANGE)	fIt->semanticBLA = "FloorSurface";
		else if (z >=  HORIZONTAL_ANGLE_RANGE)	fIt->semanticBLA = "CeilingSurface";
		else									fIt->semanticBLA = "InteriorWallSurface";
}	}	

// Assign either ceiling, floor or the default semantic based on input booleans and normal vector
void assignCeilVloor(std::set<Polyhedron::Facet_handle>& fhSet, bool canBeUp, bool canBeDown) {
	pointVector facetPoints;
	Vector_3 ortVec;
	for (std::set<Polyhedron::Facet_handle>::iterator sfIt=fhSet.begin();sfIt!=fhSet.end();++sfIt) {
		if (!canBeUp && !canBeDown) {
			(*sfIt)->semanticBLA = DEFAULT_HOR_SEMANTIC; continue;
		}

		facetPoints = comp_facetPoints(*sfIt);
		CGAL::normal_vector_newell_3(facetPoints.begin(),facetPoints.end(),ortVec);
		if (!normalizeVector(ortVec)) continue;
		if (canBeDown && ortVec.z() <= -HORIZONTAL_ANGLE_RANGE )	(*sfIt)->semanticBLA = DEFAULT_DOWN_SEMANTIC;
		else if (canBeUp && ortVec.z() >= HORIZONTAL_ANGLE_RANGE )	(*sfIt)->semanticBLA = DEFAULT_UP_SEMANTIC;
		else														(*sfIt)->semanticBLA = DEFAULT_HOR_SEMANTIC;
	}
}

// Search whether a facet is touching a certain semantic directly or indirectly through the facets in the list
bool indirectlyTouchingFindSem(std::string findSem,std::set<Polyhedron::Facet_handle>& fhSet) {	
	for (std::set<Polyhedron::Facet_handle>::iterator sfIt=fhSet.begin();sfIt!=fhSet.end();++sfIt) {
		Polyhedron::Facet::Halfedge_around_facet_circulator itHDS = (*sfIt)->facet_begin();
		do	if (itHDS->opposite()->facet()->semanticBLA == findSem)	return true;		
		while (++itHDS != (*sfIt)->facet_begin());
	}
	return false;
}

// Creates a List of all connected facets based on sem
void connectedSemFacets(Polyhedron::Facet_handle fh, std::string sem,bool allExceptThisSem, std::set<Polyhedron::Facet_handle>& fhSet, bool checkCoPlanar) {
	if (sem=="-1") sem = fh->semanticBLA;														// Set semantic to input facet sem
	fhSet.insert(fh);																			// Add to list

	Polyhedron::Facet::Halfedge_around_facet_circulator itHDS = fh->facet_begin();				// Loop over the edges
	do {
		if ((itHDS->opposite()->facet()->semanticBLA==sem ^ allExceptThisSem) &&
				(!checkCoPlanar ||  is_NewelCoplanar(itHDS))){										// If part of sem(s) we're looking for (and coplanar if needed)
			if (fhSet.find(itHDS->opposite()->facet())==fhSet.end())							// If not yet in set
				connectedSemFacets(itHDS->opposite()->facet(),sem,allExceptThisSem,fhSet,checkCoPlanar);// Recursive search
		}
	} while (++itHDS != fh->facet_begin());
}

// Depricated
// Creates a list of coplanar facets with equal semantics
// Returns the combined area (Assumes area has been set)
double coplanarSem(Polyhedron::Facet_handle fh, std::set<Polyhedron::Facet_handle>& fhSet) {
	fhSet.insert(fh);																			// Add to list
	double summedArea = fh->area;																// Get area, assumes precalculated
	Polyhedron::Facet::Halfedge_around_facet_circulator itHDS = fh->facet_begin();	
	do {
		if (is_coplanar(itHDS,true)) {															// Check coplanearity and equal semantics
			if (fhSet.find(itHDS->opposite()->facet())==fhSet.end())							// If new to list
				summedArea += coplanarSem(itHDS->opposite()->facet(),fhSet);					// Recursive search and sum
		}
	} while (++itHDS != fh->facet_begin());
	return summedArea;
}

// Remove small triangles with different semantics
void remove_singularSemantics(Polyhedron& exteriorPolyhe) {
	double SMALL_TRIANGLE_LIMIT = 0.1;
	Polyhedron::Facet_iterator exfIt;						// Iterate over exterior faces

	while (true) {
		bool newSemFound = false;
		for (exfIt = exteriorPolyhe.facets_begin(); exfIt != exteriorPolyhe.facets_end(); ++exfIt) {
			if (exfIt->area < SMALL_TRIANGLE_LIMIT) {
				std::map<std::string,double> semCountList;
				std::map<std::string,double>::iterator semCount;
				Polyhedron::Facet::Halfedge_around_facet_circulator itHDS = exfIt->facet_begin();

				double maxArea = 0;
				std::string newSem;
						
					do {
						if (is_coplanar(itHDS,false)) {
							std::string otherSem = itHDS->opposite()->facet()->semanticBLA;

							// Compute sum of area

							if (is_coplanar(itHDS->opposite()->next(),true) ||
								is_coplanar(itHDS->opposite()->prev(),true)) {
								semCount = semCountList.find(otherSem);
								if (semCount == semCountList.end())
									semCountList[otherSem] = 1; // Might not be needed
								else
									++(semCountList[otherSem]);
							}
						}
					} while (++itHDS != exfIt->facet_begin());
				
					for (semCount = semCountList.begin();semCount!=semCountList.end();++semCount) {
						if (semCount->second > maxArea) {
							maxArea = semCount->second;
							newSem = semCount->first;
						} else if (semCount->second == maxArea &&
							((exfIt->semanticBLA=="Anything"&&semCount->first!="Anything")||semCount->first==exfIt->semanticBLA))
							newSem = semCount->first;
					}
					
				if (maxArea > 0 && newSem!=exfIt->semanticBLA) {
					exfIt->semanticBLA = newSem;
					newSemFound = true;
					break;										// Dafuq? is this correct?
				}
			}
		}
		if (!newSemFound) break;								// Really?
	}
}
void apply_semanticRequirements(Polyhedron& exteriorPolyhe) {
	std::map<std::string,std::vector<bool>> semanticNormals;
	semanticNormals["Roof"] = std::vector<bool>(3,true);
	semanticNormals["Roof"][2] = false;		// down
	semanticNormals["Ground"] = std::vector<bool>(3,false);
	semanticNormals["Ground"][2] = true;
	semanticNormals["Ceiling"] = std::vector<bool>(3,false);
	semanticNormals["Ceiling"][2] = true;
	semanticNormals["Floor"] = std::vector<bool>(3,false);
	semanticNormals["Floor"][0] = true;
	semanticNormals["Wall"] = std::vector<bool>(3,true);
	semanticNormals["Window"] = std::vector<bool>(3,true);
	semanticNormals["Door"] = std::vector<bool>(3,true);
	semanticNormals["Closure"] = std::vector<bool>(3,true);
	semanticNormals[TO_DIST_SEMANTIC] = std::vector<bool>(3,true);
	//semanticNormals["Anything"] = std::vector<bool>(3,true);

	std::map<std::string,int> semanticEnum;
	semanticEnum["Roof"]		= 1;
	semanticEnum["Window"]		= 2;
	semanticEnum["Door"]		= 3;
	semanticEnum["Wall"]		= 4;
	semanticEnum["Ground"]		= 5;
	semanticEnum["Ceiling"]		= 6;
	semanticEnum["Floor"]		= 7;
	semanticEnum["Closure"]		= 8;

	semanticEnum["Anything"]	= 9;
	//semanticEnum["Install"]		= 10;
	
	Vector_3 ortVec;
	
	Polyhedron::Facet_iterator exfIt;						// Iterate over exterior faces
	for (exfIt = exteriorPolyhe.facets_begin(); exfIt != exteriorPolyhe.facets_end(); ++exfIt) {	
			
		int minSem = 99;
			
		for (std::vector<std::string>::iterator svIt=exfIt->equidistSems.begin();svIt!=exfIt->equidistSems.end();++svIt) {				// Add equidist semantics
			std::map<std::string,int>::iterator seIt = semanticEnum.find(*svIt);
			if (seIt!=semanticEnum.end()) {				// ......
				if (seIt->second<minSem) {
					minSem = seIt->second;
					exfIt->semanticBLA = *svIt;
				}
			}else {										// This seems wrong...
				exfIt->semanticBLA = *svIt;
				break;
			}
		}

		pointVector facetPoints = comp_facetPoints(exfIt);
		CGAL::normal_vector_newell_3(facetPoints.begin(),facetPoints.end(),ortVec); // Calculate normal vector, ortVec set to zero in newell
		if (!normalizeVector(ortVec)) continue;

		std::map<std::string,std::vector<bool>>::iterator  snIt = semanticNormals.find(exfIt->semanticBLA);
		if (snIt!=semanticNormals.end()) {
			if		(!snIt->second[0] && ortVec.z() > HORIZONTAL_ANGLE_RANGE)
				exfIt->semanticBLA = DEFAULT_UP_SEMANTIC;
			else if (!snIt->second[1] && ortVec.z() <= HORIZONTAL_ANGLE_RANGE && ortVec.z() >= -HORIZONTAL_ANGLE_RANGE)
				exfIt->semanticBLA = DEFAULT_HOR_SEMANTIC;
			else if (!snIt->second[2] && ortVec.z() <= -HORIZONTAL_ANGLE_RANGE)
				exfIt->semanticBLA = DEFAULT_DOWN_SEMANTIC;
		} else {
			if		(ortVec.z() > HORIZONTAL_ANGLE_RANGE)
				exfIt->semanticBLA = DEFAULT_UP_SEMANTIC;
			else if (ortVec.z() <= HORIZONTAL_ANGLE_RANGE && ortVec.z() >= -HORIZONTAL_ANGLE_RANGE)
				exfIt->semanticBLA = DEFAULT_HOR_SEMANTIC;
			else if (ortVec.z() <= -HORIZONTAL_ANGLE_RANGE)
				exfIt->semanticBLA = DEFAULT_DOWN_SEMANTIC;
		}
	}
	// Set semantics of facets created due to closing (too far from original geometry)
	for (exfIt = exteriorPolyhe.facets_begin(); exfIt != exteriorPolyhe.facets_end(); ++exfIt) {	
		if (exfIt->semanticBLA==TO_DIST_SEMANTIC) {
			std::set<Polyhedron::Facet_handle> fhSet;
			connectedSemFacets(exfIt,TO_DIST_SEMANTIC,false,fhSet);
			bool canBeUp	= indirectlyTouchingFindSem(DEFAULT_UP_SEMANTIC,fhSet);
			bool canBeDown	= indirectlyTouchingFindSem(DEFAULT_DOWN_SEMANTIC,fhSet);
			assignCeilVloor(fhSet,canBeUp,canBeDown);
		}
	}
}

struct distSemFace { 
    Kernel::FT dist;
    std::string sem;
	Polyhedron::Facet_const_handle fh;
};

struct by_dist { 
    bool operator()(distSemFace const &a, distSemFace const &b) { 
        return a.dist < b.dist;
    }
};
#include <boost/algorithm/string.hpp>
//std::vector<person> people;
//std::sort(people.begin(), people.end(), by_age());

void set_semantic_AABB_C2V(Polyhedron& exteriorPolyhe,PolVector& polyVec) {

	if (exteriorPolyhe.is_pure_triangle()) {
		std::transform( exteriorPolyhe.facets_begin(), exteriorPolyhe.facets_end(),exteriorPolyhe.planes_begin(),Plane_equation());
		std::vector<std::string>	semList;
		std::vector<std::shared_ptr<AAbbTree>>		treeList;

		// Build Trees. One for each semantic
		for(PolVector::iterator pvIt = polyVec.begin();pvIt!=polyVec.end();++pvIt) {// Get AABB trees of all semantics
			if (pvIt->is_pure_triangle()) {
				std::string semP = pvIt->facets_begin()->semanticBLA;
				std::vector<std::string>::iterator  strIt = std::find(semList.begin(), semList.end(),semP);
				if (strIt==semList.end()) {											// If new sematic
					semList.push_back(semP);										// Add sem
					std::shared_ptr<AAbbTree> tree = std::make_shared<AAbbTree>(pvIt->facets_begin(),pvIt->facets_end());		// Create tree
					tree->accelerate_distance_queries();							// accelerate
					treeList.push_back(tree);										// Add tree
				} else																					// If not new
					treeList[strIt-semList.begin()]->insert(pvIt->facets_begin(),pvIt->facets_end());	// Append to tree
			} else std::cerr << "ERROR: Not pure triangle (set_semantic_AABB2C2V)" << std::endl;
		}
		

		// For each facet calculate the least distance to each tree
		std::string semListStr = boost::algorithm::join((semList), " ");
		int percCount = 1;
		Polyhedron::Facet_iterator exfIt;						// Iterate over exterior faces
		for (exfIt = exteriorPolyhe.facets_begin(); exfIt != exteriorPolyhe.facets_end(); ++exfIt,++percCount) {	
			
			std::cout << "\r"<<semListStr<<". ("<<100*percCount/exteriorPolyhe.size_of_facets()<<"%)";

			Vector_3 orthVec = exfIt->plane().orthogonal_vector();
			normalizeVector(orthVec);	//if (!normalizeVector(ortVec)) continue;

			std::vector<distSemFace> dsfList(semList.size());										
			Point_3 centerPoint = comp_facetCentroid(exfIt);						// Compute centroid
			std::vector<Kernel::FT> leastSemDistances;
			for (int intIt=0;intIt<(int)treeList.size();++intIt) {					// Loop over all trees
				AAbbTree::Point_and_primitive_id pp = treeList[intIt]->closest_point_and_primitive(centerPoint);
				dsfList[intIt].dist	= CGAL::squared_distance(centerPoint,pp.first);	// Store distance semantic and facet for each tree
				dsfList[intIt].sem	= semList[intIt];
				dsfList[intIt].fh	= pp.second;
			}

			std::sort(dsfList.begin(),dsfList.end(),by_dist());

			exfIt->leastSqDistance = dsfList[0].dist;					// least sqrt distance
			if (exfIt->isMinkFacet = dsfList[0].dist > SEMANTIC_DISTANCE_THRESHOLD) {
				exfIt->semanticBLA = TO_DIST_SEMANTIC;						// Default semantic if too distant
				continue;
			} else
				exfIt->semanticBLA = dsfList[0].sem;					// Semantics of closest

			Vector_3 faceNormal;
			Kernel::FT faceSqArea;
			double minAngle = 10;
			Kernel::FT maxArea= 0;

			for (std::vector<distSemFace>::iterator slIt = dsfList.begin();slIt != dsfList.end();++slIt)// HANDLE ANYTHING AS LESS IMPORTANT
				if (slIt->dist < dsfList[0].dist+OVERLAP_DIST_THRESHOLD) {	// Check if Equidistant
					pointVector facetPoints = comp_facetPoints(exfIt);
					CGAL::normal_vector_newell_3(facetPoints.begin(),facetPoints.end(),faceNormal); // Calculate normal vector, ortVec set to zero in newell
					double angle = comp_angle(orthVec,faceNormal);
					if (angle!=-1 && angle < minAngle+OVERLAP_ANGLE_THRESHOLD) {
						if (minAngle >= angle+OVERLAP_ANGLE_THRESHOLD)		exfIt->equidistSems.clear();
						if (angle < minAngle)								minAngle = angle;
						faceSqArea = comp_facetSquaredArea(facetPoints);
						if (faceSqArea>maxArea-OVERLAP_AREA_THRESHOLD) {
							if (maxArea<=faceSqArea-OVERLAP_AREA_THRESHOLD)	exfIt->equidistSems.clear();
							if (faceSqArea>maxArea)							maxArea = faceSqArea;
							exfIt->equidistSems.push_back(slIt->sem);				// Add equidist semantics
						}
					}
				}
		}
		std::cout << "\r"<<semListStr<<". (100%)" << std::endl;
	}else std::cerr << "ERROR: Not pure triangle (set_semantic_AABB2C2V)" << std::endl;
}