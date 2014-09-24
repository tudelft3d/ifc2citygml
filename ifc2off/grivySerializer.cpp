/********************************************************************************
 *                                                                              *
 * This file is part of IfcOpenShell.                                           *
 *                                                                              *
 * IfcOpenShell is free software: you can redistribute it and/or modify         *
 * it under the terms of the Lesser GNU General Public License as published by  *
 * the Free Software Foundation, either version 3.0 of the License, or          *
 * (at your option) any later version.                                          *
 *                                                                              *
 * IfcOpenShell is distributed in the hope that it will be useful,              *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of               *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                 *
 * Lesser GNU General Public License for more details.                          *
 *                                                                              *
 * You should have received a copy of the Lesser GNU General Public License     *
 * along with this program. If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                              *
 ********************************************************************************/

#include "../ifcconvert/SurfaceStyle.h"
#include "grivySerializer.h"
//#include "grivyPROJECT/testHeader.h"
//#include "../stdafx.h"

bool grivySerializer::ready() {
	return off_stream.is_open() && offx_stream.is_open();
}

void grivySerializer::writeHeader() {
	//off_stream.precision(20);
}

//void grivySerializer::writeTesselated(const IfcGeomObjects::IfcGeomObject* o) {
//	
//	object_stream << o->type << " ";
//
//	for ( IfcGeomObjects::FltIt it = o->mesh->verts.begin(); it != o->mesh->verts.end(); ) {
//		const double x = *(it++);
//		const double y = *(it++);
//		const double z = *(it++);
//		coord_stream << x << " " << y << " " << z << std::endl;
//	}
//	for ( IfcGeomObjects::IntIt it = o->mesh->faces.begin(); it != o->mesh->faces.end(); ) {
//		const int v1 = *(it++)+vcount_total;
//		const int v2 = *(it++)+vcount_total;
//		const int v3 = *(it++)+vcount_total;
//		poly_stream  << v1 << " " << v2 << " " << v3 << std::endl;
//		object_stream << p_count << " ";
//		++p_count;
//	}
//	object_stream << std::endl;
//}
void grivySerializer::writeTesselated(const IfcGeomObjects::IfcGeomObject* o) {
	
	unsigned int v_count = 0, f_count = 0;
	std::stringstream vSStream;
	std::stringstream fSStream;

	std::vector<std::vector<double>> vUniqueSet;		//set unsafe, unpredictable ordering
	std::map<int,int> vRefMap;		

	for ( IfcGeomObjects::FltIt it = o->mesh->verts.begin(); it != o->mesh->verts.end(); ) {
		const double x = *(it++);				// get coords
		const double y = *(it++);
		const double z = *(it++);

		std::vector<double> vVector;			//put coords in vector
		vVector.push_back(x); vVector.push_back(y); vVector.push_back(z);
		
		int vRef_count = 0;					// determine count of first vector
		for (std::vector<std::vector<double>>::iterator vIt = vUniqueSet.begin();vIt!=vUniqueSet.end();vIt++,vRef_count++) {
			if (*vIt == vVector) break;
		}

		vRefMap[v_count] = vRef_count;							// link coords to first its first occurance
					
		if (vRef_count==vUniqueSet.size()) {					// if new vector
			vUniqueSet.push_back(vVector);						// add vector to set
			vSStream << x << " " << y << " " << z << std::endl;	//print to file
		}
		++v_count;												// increment v_count
	}
	for ( IfcGeomObjects::IntIt it = o->mesh->faces.begin(); it != o->mesh->faces.end(); ) {
		const int v1 = *(it++)+vcount_total -1 ;
		const int v2 = *(it++)+vcount_total -1 ;
		const int v3 = *(it++)+vcount_total -1 ;

		fSStream << 3 << " " << vRefMap[v1] << " " << vRefMap[v2] << " " << vRefMap[v3] << std::endl;
		++f_count;
	}
	off_stream << "OFF" << std::endl << vUniqueSet.size() << " " <<f_count << " 0" << std::endl;

// test ---------------------------------
	/*if (TEST_IS_ONNN){
		std::stringstream offSs;
		offSs << "OFF" << std::endl << vUniqueSet.size() << " " <<f_count << " 0" << std::endl;
		offSs << vSStream.str() << fSStream.str();

		testNef(offSs);
	}*/
// end test -----------------------------

	off_stream << vSStream.str() << fSStream.str();

	unsigned int next_offLine_count = offLine_count+2+vUniqueSet.size()+f_count;
	offx_stream << semantics << " "<< offLine_count << " " << next_offLine_count << std::endl;
	
	offLine_count = next_offLine_count;
}










void grivySerializer::writeShapeModel(const IfcGeomObjects::IfcGeomShapeModelObject* o) {	
	
	IfcGeom::ShapeList new_topo_shapes;
	
	for (IfcGeom::ShapeList::const_iterator it = o->mesh->begin(); it != o->mesh->end(); ++ it) {		//iterate over shapelist

		gp_GTrsf gtrsf = *it->first;			// Read specific transformation
		gtrsf.PreMultiply(o->trsf);				// Read+apply common transformation
		const TopoDS_Shape& s = *it->second;	// Read TopoDS_Shape		
			
		bool trsf_valid = false;
		gp_Trsf trsf;
		try {
			trsf = gtrsf.Trsf();				// Test transformation
			trsf_valid = true;					// Check when succesfull
		} catch (...) {}
			
		// Do Boolean ops here? no

		const TopoDS_Shape moved_shape = trsf_valid
			? BRepBuilderAPI_Transform(s, trsf, true).Shape()		// Valid transfmation
			: BRepBuilderAPI_GTransform(s, gtrsf, true).Shape();	// Invalid transfmation

		// Use to recreate IfcGeomShapeModelObject o using moved_shape
		IfcGeom::LocationShape* new_loc_shape;
		new_loc_shape = new IfcGeom::LocationShape(&gtrsf,&moved_shape);  //moved_shape / s?	
		new_topo_shapes.push_back(*new_loc_shape); // store in new_shapes
		
	}

	IfcGeomObjects::IfcRepresentationShapeModel* rep_shape;
	rep_shape = new IfcGeomObjects::IfcRepresentationShapeModel(o->mesh->getId(),new_topo_shapes);

	IfcGeomObjects::IfcGeomShapeModelObject* new_o;
	new_o = new IfcGeomObjects::IfcGeomShapeModelObject(o->id,o->parent_id,o->name,o->type,o->guid,o->trsf,rep_shape);
	////IfcGeomObjects::IfcGeomShapeModelObject(


	//// Convert to BRep	
	//IfcGeomObjects::IfcGeomBrepDataObject* brep_object;
	//brep_object = new IfcGeomObjects::IfcGeomBrepDataObject(*o);

	//brep_object->mesh->brep_data;


	//// Convert to triangulated BRep	
	IfcGeomObjects::IfcGeomObject* geom_object;
	geom_object = new IfcGeomObjects::IfcGeomObject(*new_o);
	//// Write CityGML

	writeTesselated(geom_object);
}