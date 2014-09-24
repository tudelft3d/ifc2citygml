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

#ifndef grivySERIALIZER_H
#define grivySERIALIZER_H

#include <set>
#include <string>
#include <fstream>
//#include <fstream>

#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepBuilderAPI_Transform.hxx>

#include "../ifcconvert/GeometrySerializer.h"
#include "../ifcconvert/SurfaceStyle.h"

// test ---------------------------------
//#include "grivyPROJECT/testHeader.h"
//#define TEST_IS_ONNN false
//end test--------------------------------

class grivySerializer : public GeometrySerializer {
private:
//	std::ofstream coord_stream;
//	std::ofstream poly_stream;
//	std::ofstream object_stream;

	std::ofstream off_stream;
	std::ofstream offx_stream;
	unsigned int offLine_count;

	unsigned int vcount_total;
	unsigned int p_count;

public:
	grivySerializer(const std::string& out_filename)
		: GeometrySerializer()
//		,  coord_stream((out_filename + ".co" ).c_str())
//		,   poly_stream((out_filename + ".po" ).c_str())
//		, object_stream((out_filename + ".ob" ).c_str())
		,    off_stream((out_filename + ".OFF").c_str())
		,    offx_stream((out_filename + ".OFFx").c_str())
		, offLine_count(0)
		, vcount_total(1)
		, p_count(0)
	{}
	virtual ~grivySerializer() {}
	bool ready();
	void writeHeader();
	void writeMaterial(const SurfaceStyle& style);
	void writeTesselated(const IfcGeomObjects::IfcGeomObject* o);
	void writeShapeModel(const IfcGeomObjects::IfcGeomShapeModelObject* o);
	void finalize() {}
	bool isTesselated() const { return true; } // original true
};

#endif