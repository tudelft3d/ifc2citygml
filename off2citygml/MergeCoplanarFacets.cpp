#include "MergeCoplanarFacets.h"


pointVector get_facetPoints(Polyhedron::Halfedge_handle& heH) {
	Polyhedron::Facet::Halfedge_around_facet_circulator itHDS = heH->facet_begin(); // same as just heH?
	pointVector facetPnts;
	do facetPnts.push_back(itHDS->vertex()->point());
	while (++itHDS != heH->facet_begin());
	return facetPnts;
}

// Checks if there are two disconnected linestrings which both have the 2 coplanar facets
bool joinCreatesNoHole (Polyhedron::Halfedge_handle& heH) {
	pointVector otherFacetPnts = get_facetPoints(heH);		// Get points of other facet

	Polyhedron::Halfedge_around_facet_circulator hafIt = heH->opposite()->facet_begin();
	bool nextNotBorder	= false;				// starts with heH so false
	bool difFound		= false;				// if previous was not border
	bool refound		= false;				// if two border sections found with not border in between
	bool canJoinFacets	= true;					// output
	do {
		if (difFound ^ nextNotBorder) {			// heH->facet() != hafIt->opposite()->facet()) {
			if (refound) {
				canJoinFacets=false;			//if refound twice -> discconnected borders
				break;
			}
			refound		=  difFound;			// Set refound true
			difFound	= !difFound;			// Flip difFound
		}
		nextNotBorder = heH->facet() != hafIt->next()->opposite()->facet();
		if (difFound && nextNotBorder &&		// if current and next are not border and point is on other facet
			std::find(otherFacetPnts.begin(), otherFacetPnts.end(),hafIt->vertex()->point())!=otherFacetPnts.end()) {
				canJoinFacets=false;			// Point is touching
				break;
		}
	} while (++hafIt != heH->opposite()->facet_begin());
	return canJoinFacets;
}
// Checks if there are two disconnected linestrings which both have the 2 coplanar facets
bool joinCreatesNoHole2 (Polyhedron::Halfedge_handle& heH) {
	return heH->facet() != heH->opposite()->facet();
}

bool triDoesNotIntersectFacet (Polyhedron::Halfedge_handle& heH, bool isOpposite) {

	Segment_3 checkSeg (heH->vertex()->point(),heH->prev()->prev()->vertex()->point());	
	Polyhedron::Halfedge_around_facet_circulator hafIt = heH->facet_begin()++;			// skip itself and first

	while (++hafIt != heH->prev()->facet_begin()) {
		if (CGAL::do_intersect(checkSeg,Segment_3(hafIt->vertex()->point(),hafIt->prev()->vertex()->point())))
			return false;
	} 
	if (!isOpposite)
		return triDoesNotIntersectFacet (heH->opposite(), true);
	else
		return true;
}

void mergeColinear(Polyhedron& p) {
	int vertBefore =  p.size_of_vertices();
	bool colinearFound = true;
	while (colinearFound) {
		colinearFound = false; // Set temporarily to false, if merge degments then set true

		int percCount = 1;
		for (Polyhedron::Halfedge_iterator hit = p.halfedges_begin(); hit != p.halfedges_end(); ++hit,++percCount){
			if (CGAL::circulator_size(hit->vertex_begin()) == 1) {
				std::cerr << "WARNING: Loose edge, but how??"<<std::endl;
				while (CGAL::circulator_size(hit->vertex_begin()) == 1)
					hit = p.join_vertex(hit->opposite());
				break;
			}

			if ((CGAL::circulator_size(hit->vertex_begin()) == 2) &&			// if only two he connected to vertex
				(hit->facet_degree()>3 && hit->opposite()->facet_degree()>3)) { // if faces are not triangles // prob faster

				Vector_3 cur(hit->prev()->vertex()->point(),hit->vertex()->point());
				Vector_3 nex(hit->vertex()->point(),hit->next()->vertex()->point());
				if ( is_colinear(cur,nex)) {									// check if colinear
					std::cout << "\rVertices before/after: "<<vertBefore<<" -> "<< p.size_of_vertices()<<". ("<<100*percCount/p.size_of_halfedges()<<"%)";
					p.join_vertex(hit->opposite());								// move cur to prev point
					colinearFound = true;
					break;
	}	}	}	}
	std::cout << "\rVertices before/after: "<<vertBefore<<" -> "<< p.size_of_vertices()<<". (100%)" << std::endl;
}


// Should group triangles first before checking adjacency and merging! Now, although the normal is not recalculated it can still kinda propegate.

void mergeCoplanar(Polyhedron& p,bool step2) {
	int facetsBefore =  p.size_of_facets();
	
	p.normalize_border();
	if(!p.size_of_border_halfedges()) {
		// Calculate normals only once in advace! so all tris should be coplanar with the original
		std::transform( p.facets_begin(), p.facets_end(),p.planes_begin(),Plane_equation());			// Calculate plane equations (only works on tri<- = bs)=true
		bool coplanarFound = true;
		std::vector<Polyhedron::Halfedge_handle> skipHEs;
		while (coplanarFound) {			
			coplanarFound = false;																		// Set coplanarFound false
			int percCount = 1;
			for (Polyhedron::Halfedge_iterator hit = p.halfedges_begin(); hit != p.halfedges_end(); ++hit,++percCount){ // Loop through all halfedges
				if (is_coplanar(hit,true)){																// If coplanar and equals semantics
					Polyhedron::Halfedge_handle removeMe = hit;
					while (CGAL::circulator_size(removeMe->vertex_begin()) < 3)							// Mover handle to beginning of linestring
						removeMe = removeMe->next();
					
					bool jcnh = false;
					if (!step2) jcnh = joinCreatesNoHole (hit);
					else		jcnh = joinCreatesNoHole2(hit);
					if (jcnh){														// If no holes will be created

						std::cout << "\rFacets   before/after: "<<facetsBefore<<" -> "<< p.size_of_facets()<<". ("<<100*percCount/p.size_of_halfedges()<<"%)";

						while (CGAL::circulator_size(removeMe->opposite()->vertex_begin()) < 3)			// Join vertexes until at the other end of linestring
							if (removeMe->facet_degree()>3 && removeMe->opposite()->facet_degree()>3)
								removeMe = (p.join_vertex(removeMe))->next()->opposite();
							else																		// One of the faces turned into a triangle ->remove center vertex
								break;																	
						if (CGAL::circulator_size(removeMe->opposite()->vertex_begin()) < 3)			// Remove remained of the border
							p.erase_center_vertex(removeMe->opposite());								// if two segments remain remove center point
						else
							p.join_facet(removeMe);														// if one segment remains join facets
						coplanarFound = true;
						break;
					} else { // simplify border, but how to do this safely? not optimal solution implemented. Should do: add inward offseted point of intersection etc.
						if (std::find(skipHEs.begin(), skipHEs.end(),hit)!=skipHEs.end()) {					// Skip if hit in skipList
							while (CGAL::circulator_size(removeMe->opposite()->vertex_begin()) < 3)	{		// Join vertexes until at the other end of linestring
								if (removeMe->facet_degree()>3 && removeMe->opposite()->facet_degree()>3)
									if (triDoesNotIntersectFacet(removeMe))									// if tri reMe,reME->prev does not intersect left or right facet
										removeMe = (p.join_vertex(removeMe))->next()->opposite();			// remove removeME
									else {
										skipHEs.push_back(removeMe);
										skipHEs.push_back(removeMe->opposite());
										removeMe = removeMe->prev();										// move removeME one halfedge back
									}
								else break;																	// stop if only a triangle remains or at other end																
							}	
							skipHEs.push_back(removeMe);
							skipHEs.push_back(removeMe->opposite());
	}	}	}	}	}	}
	//if (!step2) mergeCoplanar(p,true);
	//else 
		std::cout << "\rFacets   before/after: "<<facetsBefore<<" -> "<< p.size_of_facets()<<". (100%)"<<std::endl;
}