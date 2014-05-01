#include "CommonWriters.h"



std::vector<double> pushColrs(double r, double g, double b, double a){
	std::vector<double> colr;
	colr.push_back(r);colr.push_back(g);colr.push_back(b);colr.push_back(a);
	return colr;
}

std::vector<double> randomColr() {
	return pushColrs((rand()%255)/255.0,(rand()%255)/255.0,(rand()%255)/255.0,1);
}

std::map<std::string,std::vector<double>> getColorMap () {
	std::map<std::string,std::vector<double>> mp;
	mp["Wall"]			= pushColrs(0.8,0.8,0.8,1);
	mp["Roof"]			= pushColrs(1,0,0,1);
	mp["Ceiling"]		= pushColrs(0.2,1,0.2,1);
	mp["Floor"]			= pushColrs(0,1,1,1);
	mp["Ground"]		= pushColrs(1,1,0.4,1);
	mp["Window"]		= pushColrs(0,0.4,1,0.2);
	mp["Door"]			= pushColrs(0.5,0.165,0.165,1);
	mp["Closure"]		= pushColrs(1,0.3,0,0.5);
	mp["Install"]		= pushColrs(0.8,0.7,0.6,1);
	return  mp;
}

std::string fn2offFN(std::string& filename) {
	if (!(filename.size()>2 && filename[1]==':'))
		filename = "off\\" + filename;
	return filename;
}

bool write_polyhe2ColoredOFF(Polyhedron& polyhe, std::string filename_0ext) {

	fn2offFN(filename_0ext);

	std::map<std::string,std::vector<double>> colrMap = getColorMap (); //temp

	std::ofstream off_ofstream(filename_0ext +".OFF");	// Open off file
	off_ofstream.precision(OUTPUT_DECIMALS);
	if (off_ofstream.is_open())	{						// Check if open

		// writes P to `out' in the format provided by `writer'.
		Polyhedron::Vertex_const_iterator                  vi;
		Polyhedron::Facet_const_iterator                   fi;
		Polyhedron::Halfedge_around_facet_const_circulator hc,hc_end;
		// Print header.
		off_ofstream << "OFF" << std::endl;
		off_ofstream << polyhe.size_of_vertices()<<" "<< polyhe.size_of_facets() << " 0" <<std::endl;

		for( vi = polyhe.vertices_begin(); vi != polyhe.vertices_end(); ++vi) {
			off_ofstream	<< CGAL::to_double( vi->point().x()) << " "
							<< CGAL::to_double( vi->point().y()) << " "
							<< CGAL::to_double( vi->point().z()) <<std::endl;
		}
		typedef CGAL::Inverse_index< Polyhedron::Vertex_const_iterator> Index;
		Index index( polyhe.vertices_begin(), polyhe.vertices_end());

		for( fi = polyhe.facets_begin(); fi != polyhe.facets_end(); ++fi) {
			hc = fi->facet_begin();
			hc_end = hc;
			std::size_t n = CGAL::circulator_size( hc);
			CGAL_assertion( n >= 3);
			off_ofstream	<< n << " ";	// number of vertices
			do {
				off_ofstream	<< index[ Polyhedron::Vertex_const_iterator(hc->vertex())] << " ";
				++hc;
			} while( hc != hc_end);

			std::vector<double> colr;					// Find color for semantic
			std::string sem = fi->semanticBLA;
			std::map<std::string,std::vector<double>>::iterator colrIt = colrMap.find(sem);
			if (colrIt !=colrMap.end())	colr = colrMap[sem];
			else {										// Define random color for new semantic
				colr = randomColr();
				colrMap[sem] = colr;
			}

			for( std::vector<double>::const_iterator i = colr.begin(); i != colr.end(); ++i) {
				off_ofstream << *i;
				if (i != colr.end())	off_ofstream << " ";
			}
			off_ofstream << std::endl;
		}

		off_ofstream.close();							// Close file
		return true;
	} else std::cerr << "WARNING: Can't open file: " << filename_0ext << ".OFF" <<std::endl;
	return false;
}

void polyheWriteStats ( Polyhedron polyhe, std::string filename_0ext, CGAL::Timer& tmr, int stage, bool lastStage) {
	tmr.stop();
	std::cout << "Time: " << tmr.time() << std::endl;// << " facets: " <<polyhe.size_of_facets() <<" vertexes: " << polyhe.size_of_vertices() 
	std::string stagestr;
	switch (stage) {
	case 1: stagestr = "_1grow"; break;
	case 2: stagestr = "_2exterior"; break;
	case 3: stagestr = "_3shrink"; break;
	}

	write_polyhe2off(polyhe,filename_0ext+stagestr);
	writeLog ( filename_0ext,tmr,stagestr,lastStage);
	tmr.reset(); tmr.start();
}

void nefWriteStats ( Nef_polyhedron nefPolyhe, std::string filename_0ext, CGAL::Timer& tmr, int stage, bool lastStage) {
	tmr.stop();
	std::cout << "Time: " << tmr.time()<< std::endl;// << " facets: " <<nefPolyhe.number_of_facets() <<" vertexes: " << nefPolyhe.number_of_vertices() << std::endl;
	std::string stagestr;
	switch (stage) {
	case 1: stagestr = "_1grow"; break;
	case 2: stagestr = "_2exterior"; break;
	case 3: stagestr = "_3shrink"; break;
	}
	write_nef2off(nefPolyhe,filename_0ext+stagestr);
	writeLog ( filename_0ext,tmr,stagestr,lastStage);
	tmr.reset(); tmr.start();
}

void writeLog (std::string filename_0ext, CGAL::Timer& tmr, std::string stagestr, bool lastStage) {

	std::string line = "";
	std::ifstream off_ifstream(filename_0ext+stagestr +".OFF");
	if (off_ifstream.is_open()) {								
												
		std::getline(off_ifstream,line);
		std::getline(off_ifstream,line);
		off_ifstream.close();
		line.pop_back();
	} else std::cerr << "WARNING: Can't open file: " << filename_0ext+stagestr << ".OFF" <<std::endl;	
	
	std::ofstream outfile(filename_0ext+".log", std::ios_base::app);
	outfile << tmr.time() <<" "<< line;
	if (lastStage) outfile << std::endl; 
	outfile.close();
}


/* ------------------- Writer functions ---------------------------*/

// Writes Polyhedron to 'filename_0ext +".OFF"'
bool write_polyhe2off(Polyhedron& polyhe, std::string filename_0ext) {
	fn2offFN(filename_0ext);
	std::ofstream off_ofstream(filename_0ext +".OFF");	// Open off file
	off_ofstream.precision(OUTPUT_DECIMALS);
	if (off_ofstream.is_open())	{						// Check if open
		off_ofstream << polyhe;							// Write polyhe
		off_ofstream.close();							// Close file
		return true;
	}
	else std::cerr << "WARNING: Can't open file: " << filename_0ext << ".OFF" <<std::endl;
	return false;
}

// Writes Nef_polyhedron to 'filename_0ext +".OFF"'
// Converts nef to polyhe then calls write_polyhe2off
// Nef should be simple
bool write_nef2off(Nef_polyhedron& nefPolyhe, std::string filename_0ext) {
	if (nefPolyhe.is_simple()) {						// Check if nef is simple
		Polyhedron polyhe;								// Init polyhe
		
		nef2polyhe ( nefPolyhe , polyhe );
		//nefPolyhe.convert_to_polyhedron(polyhe);		// Convert nef to polyhe
		return write_polyhe2off(polyhe,filename_0ext);	// Call to write polyhe to off
	} else std::cerr << "ERROR: not simple"<<std::endl;
	return false;
}