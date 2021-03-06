﻿/********************************************************************************
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

/********************************************************************************
 *                                                                              *
 * This started as a brief example of how IfcOpenShell can be interfaced from   * 
 * within a C++ context, it has since then evolved into a fullfledged command   *
 * line application that is able to convert geometry in an IFC files into       *
 * several tesselated and topological output formats.                           *
 *                                                                              *
 ********************************************************************************/

#include <fstream>
#include <sstream>
#include <set>
#include <time.h>

#include <boost/program_options.hpp>
#define BOOST_PROGRAM_OPTIONS_NO_LIB 1   // remove this?

#include "../ifcgeom/IfcGeomObjects.h"

#include "../ifcconvert/SurfaceStyle.h"
//#include "../ifcconvert/ColladaSerializer.h"
#include "../ifcconvert/IgesSerializer.h"
#include "../ifcconvert/StepSerializer.h"
#include "../ifcconvert/WavefrontObjSerializer.h"
//#include "../ifcconvert/VRMLgrivySerializer.h"
#include "../ifcconvert/CityGMLgrivySerializer.h"
#include "../grivy/grivySerializer.h"
#include "../grivy/CityGMLSemantics.h"



void printVersion() {
	std::cerr << "IfcOpenShell IfcConvert " << IFCOPENSHELL_VERSION << std::endl;
}

void printUsage(const boost::program_options::options_description& generic_options, const boost::program_options::options_description& geom_options) {
	printVersion();
	std::cerr << "Usage: IfcConvert [options] <input.ifc> [<output>]" << std::endl
	          << std::endl
	          << "Converts the geometry in an IFC file into one of the following formats:" << std::endl
	          << "  .obj   WaveFront OBJ  (a .mtl file is also created)" << std::endl
//	          << "  .dae   Collada        Digital Assets Exchange" << std::endl
	          << "  .stp   STEP           Standard for the Exchange of Product Data" << std::endl
	          << "  .igs   IGES           Initial Graphics Exchange Specification" << std::endl
//			  << "  .vrml  VRML           Virtual Reality Modeling Language" << std::endl
			  << "  .gml   CityGML        City Geography Markup Language" << std::endl
	          << std::endl
	          << "Command line options" << std::endl << generic_options << std::endl
	          << "Advanced options" << std::endl << geom_options << std::endl;
}

std::string change_extension(const std::string& fn, const std::string& ext) {
	std::string::size_type dot = fn.find_last_of('.');
	if (dot != std::string::npos) {
		return fn.substr(0,dot+1) + ext;
	} else {
		return fn + "." + ext;
	}
}

int main(int argc, char** argv) {
	boost::program_options::options_description generic_options;
	generic_options.add_options()
		("help", "display usage information")
		("version", "display version information")
		("verbose,v", "more verbose output");

	boost::program_options::options_description fileio_options;
	fileio_options.add_options()
		("input-file", boost::program_options::value<std::string>(), "input IFC file")
		("output-file", boost::program_options::value<std::string>(), "output geometry file");

	std::vector<std::string> ignore_types_vector;
	boost::program_options::options_description geom_options;
	geom_options.add_options()
		("weld-vertices", "Specifies whether vertices are welded, meaning that the coordinates "
						  "vector will only contain unique xyz-triplets. This results in a "
					      "manifold mesh which is useful for modelling applications, but might "
					      "result in unwanted shading artifacts in rendering applications.")
		("use-world-coords", "Specifies whether to apply the local placements of building elements "
						     "directly to the coordinates of the representation mesh rather than "
						     "to represent the local placement in the 4x3 matrix, which will in that "
						     "case be the identity matrix.")
		("convert-back-units", "Specifies whether to convert back geometrical output back to the "
							   "unit of measure in which it is defined in the IFC file. Default is "
							   "to use meters.")
		("sew-shells", "Specifies whether to sew the faces of IfcConnectedFaceSets together. This is a "
					   "potentially time consuming operation, but guarantees a consistent orientation "
					   "of surface normals, even if the faces are not properly oriented in the IFC file.")
		("merge-boolean-operands", "Specifies whether to merge all IfcOpening operands into a single"
								   "operand before applying the subtraction operation. This may "
								   "introduce a performance improvement at the risk of failing, in "
								   "which case the subtraction is applied one-by-one.")
		("force-ccw-face-orientation", "Recompute topological face normals using Newell's Method to "
									   "guarantee that face vertices are defined in a Counter Clock "
									   "Wise order, even if the faces are not part of a closed shell.")
		("disable-opening-subtractions", "Specifies whether to disable the boolean subtraction of "
										 "IfcOpeningElement Representations from their RelatingElements.")
		("ignore-types", boost::program_options::value< std::vector<std::string> >(&ignore_types_vector)->multitoken(), 
			"A list of IFC datatype keywords that should not be included in the geometrical output");
	
	boost::program_options::options_description cmdline_options;
	cmdline_options.add(generic_options).add(fileio_options).add(geom_options);

	boost::program_options::positional_options_description positional_options;
	positional_options.add("input-file", 1);
	positional_options.add("output-file", 1);

	boost::program_options::variables_map vmap;
	boost::program_options::store(boost::program_options::command_line_parser(argc, argv).
			  options(cmdline_options).positional(positional_options).run(), vmap);
	boost::program_options::notify(vmap);

	if (vmap.count("help") || !vmap.count("input-file")) {
		printUsage(generic_options, geom_options);
		return 1;
	} else if (vmap.count("version")) {
		printVersion();
		return 1;
	}

	const bool verbose = vmap.count("verbose") != 0;
	const bool weld_vertices = vmap.count("weld-vertices") != 0;
	const bool use_world_coords = vmap.count("use-world-coords") != 0;
	const bool convert_back_units = vmap.count("convert-back-units") != 0;
	const bool sew_shells = vmap.count("sew-shells") != 0;
	const bool merge_boolean_operands = vmap.count("merge-boolean-operands") != 0;
	const bool force_ccw_face_orientation = vmap.count("force-ccw-face-orientation") != 0;
	const bool disable_opening_subtractions = vmap.count("disable-opening-subtractions") != 0;	

	// Gets the set ifc types to be ignored from the command line. 
	std::set<std::string> ignore_types;
	for (std::vector<std::string>::const_iterator it = ignore_types_vector.begin(); it != ignore_types_vector.end(); ++it) {
		std::string lowercase_type = *it;
		for (std::string::iterator c = lowercase_type.begin(); c != lowercase_type.end(); ++c) {
			*c = tolower(*c);
		}
		ignore_types.insert(lowercase_type);
	}
	// If none are specified these are the defaults to skip from output
	if (ignore_types_vector.empty()) {
		ignore_types.insert("ifcopeningelement");
		//ignore_types.insert("ifcspace");	// needed for closure
	}
	
	const std::string input_filename = vmap["input-file"].as<std::string>();
	// If no output filename is specified a Wavefront OBJ file will be output
	// to maintain backwards compatibility with the obsolete IfcObj executable.
	const std::string output_filename = vmap.count("output-file") == 1 
		? vmap["output-file"].as<std::string>()
		: change_extension(input_filename, "obj");
	
	if (output_filename.size() < 5) {
		printUsage(generic_options, geom_options);
		return 1;
	}

	IfcGeomObjects::Settings(IfcGeomObjects::USE_WORLD_COORDS, use_world_coords);
	IfcGeomObjects::Settings(IfcGeomObjects::WELD_VERTICES, weld_vertices);
	IfcGeomObjects::Settings(IfcGeomObjects::SEW_SHELLS, sew_shells);	

	std::string output_extension = output_filename.substr(output_filename.size()-4);
	for (std::string::iterator c = output_extension.begin(); c != output_extension.end(); ++c) {
		*c = tolower(*c);
	}

	GeometrySerializer* serializer;
	if (output_extension == ".obj") {
		const std::string mtl_filename = output_filename.substr(0,output_filename.size()-3) + "mtl";
		if (!use_world_coords) {
			Logger::Message(Logger::LOG_WARNING, "Use world coord settings ignored for WaveFront OBJ files");
			IfcGeomObjects::Settings(IfcGeomObjects::USE_WORLD_COORDS, true);
		}
		serializer = new WaveFrontOBJSerializer(output_filename, mtl_filename);
	//} else if (output_extension == ".dae") {
	//	serializer = new ColladaSerializer(output_filename);
	} else if (output_extension == ".stp") {
		serializer = new StepSerializer(output_filename);
	} else if (output_extension == ".igs") {
		// Not sure why this is needed, but it is.
		// See: http://tracker.dev.opencascade.org/view.php?id=23679
		IGESControl_Controller::Init();
		serializer = new IgesSerializer(output_filename);
	//} else if (output_extension == ".vrml") {
	//	serializer = new VRMLgrivySerializer(output_filename);
	} else if (output_extension == ".gml") {
		serializer = new CityGMLgrivySerializer(output_filename); /////// CHANGE to CityGMLgrivySERIALIZER
	} else if (output_extension == ".tmp") {
		if (!use_world_coords) {
			Logger::Message(Logger::LOG_WARNING, "Use world coord settings ignored for tmp files");
			IfcGeomObjects::Settings(IfcGeomObjects::USE_WORLD_COORDS, true);
		}
		serializer = new grivySerializer(output_filename); 
	} else {
		Logger::Message(Logger::LOG_ERROR, "Unknown output filename extension");
		printUsage(generic_options, geom_options);
		return 1;
	}

	Logger::Verbosity(verbose ? Logger::LOG_NOTICE : Logger::LOG_ERROR);

	if (!serializer->ready()) {
		Logger::Message(Logger::LOG_ERROR, "Unable to open output file for writing");
		return 1;
	}

	serializer->writeHeader(); // write header

	if (!serializer->isTesselated()) {
		IfcGeomObjects::Settings(IfcGeomObjects::DISABLE_TRIANGULATION, true);
	}

	// Stream for log messages, we don't want to interupt our new progress bar...
	std::stringstream ss;

	// Parse the file supplied in argv[1]. Returns true on succes.
	if ( ! IfcGeomObjects::Init(input_filename,&std::cout,&ss) ) {
		Logger::Message(Logger::LOG_ERROR, "Unable to parse .ifc file or no geometrical entities found");
		return 1;
	}


	std::vector<semanticMAP> smv = read_semanticSettings();	// Read semantic settings from 'IFC2CityGML_Settings.ini'
	semanticMAP smapONLY = smv[0];
	semanticMAP smapANY  = smv[1];
	semanticMAP smapPART = smv[2];

	std::set<std::string> materials;

	time_t start,end;
	time(&start);
	int old_progress = -1;
	Logger::Status("Creating geometry...");

	// The functions IfcGeomObjects::Get() and IfcGeomObjects::Next() wrap an iterator of all geometrical entities in the Ifc file.
	// IfcGeomObjects::Get() returns an IfcGeomObjects::IfcGeomObject (see IfcGeomObjects.h for definition)
	// IfcGeomObjects::Next() is used to poll whether more geometrical entities are available    
	do {
		const IfcGeomObjects::IfcObject* geom_object;
		
		if (serializer->isTesselated()) {
			geom_object = IfcGeomObjects::Get();
		} else {
			geom_object = IfcGeomObjects::GetShapeModel(); //ifc_product->entity->id(), parent_id, name, Ifc2x3::Type::ToString(ifc_product->type()), guid, trsf, shape
		}
		
		std::string lowercase_type = geom_object->type;			// type to lower case
		for (std::string::iterator c = lowercase_type.begin(); c != lowercase_type.end(); ++c) 
			*c = tolower(*c);
		if (ignore_types.find(lowercase_type) != ignore_types.end()) continue;	// Check whether type should be ignored
// ADD CHECK HERE TO CHECK WHETHER relType or AGGREGATE TYPE SHOULD BE IGNORED

		const Ifc2x3::IfcObjectDefinition::ptr objectDef = static_cast<Ifc2x3::IfcObjectDefinition*>(IfcGeomObjects::GetFile()->EntityById(geom_object->id));
		std::string semantics = get_semantic(objectDef, smapONLY,smapANY,smapPART);		

		serializer->setCurrentSemantics(semantics);
		 
		//const Ifc2x3::IfcBuildingElement::ptr element = static_cast<const Ifc2x3::IfcBuildingElement*>(geom_object);

		if (serializer->isTesselated()) {
			serializer->writeTesselated(static_cast<const IfcGeomObjects::IfcGeomObject*>(geom_object));
		} else {
			serializer->writeShapeModel(static_cast<const IfcGeomObjects::IfcGeomShapeModelObject*>(geom_object)); // cast and then start write function
		}

		const int progress = IfcGeomObjects::Progress() / 2;
		if ( old_progress!= progress ) Logger::ProgressBar(progress);
		old_progress = progress;
	
	} while ( IfcGeomObjects::Next() );

	serializer->finalize();
	delete serializer;

	Logger::Status("\rDone creating geometry                                ");

	// Writes the material settings, defined in Materials.h
	std::string log = ss.str();
	if (!log.empty()) {
		std::cerr << std::endl << "Log:" << std::endl;
		std::cerr << ss.str();
	}

	time(&end);
	int dif = (int) difftime (end,start);	
	printf ("\nExplicit geometry & intermediate semantics in: %d seconds\n", dif );
}

SurfaceStyle GetDefaultMaterial(const std::string& s) {
	if (s == "IfcSite"            ) { return SurfaceStyle("IfcSite",            0.75,0.8,0.65); }
	if (s == "IfcSlab"            ) { return SurfaceStyle("IfcSlab",            0.4, 0.4,0.4 ); }
	if (s == "IfcWallStandardCase") { return SurfaceStyle("IfcWallStandardCase",0.9, 0.9,0.9 ); }
	if (s == "IfcWall"            ) { return SurfaceStyle("IfcWall",            0.9, 0.9,0.9 ); }
	if (s == "IfcWindow"          ) { return SurfaceStyle("IfcWindow",          0.75,0.8,0.75, 1.0,1.0,1.0, 0.0,0.0,0.0, 500.0, 0.3); }
	if (s == "IfcDoor"            ) { return SurfaceStyle("IfcDoor",            0.55,0.3,0.15); }
	if (s == "IfcBeam"            ) { return SurfaceStyle("IfcBeam",            0.75,0.7,0.7 ); }
	if (s == "IfcRailing"         ) { return SurfaceStyle("IfcRailing",         0.65,0.6,0.6 ); }
	if (s == "IfcMember"          ) { return SurfaceStyle("IfcMember",          0.65,0.6,0.6 ); }
	if (s == "IfcPlate"           ) { return SurfaceStyle("IfcPlate",           0.8, 0.8,0.8 ); }
	return SurfaceStyle(s);
}
