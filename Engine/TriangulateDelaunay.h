// Delaunay triangulation algorith implementation based on concepts on http://www.codeguru.com/cpp/cpp/algorithms/general/article.php/c8901/Delaunay-Triangles.htm
// and http://en.wikipedia.org/wiki/Delaunay_triangulation
//
//
//


#ifndef LEVIATHAN_TRIANGLEDELAUNAY
#define LEVIATHAN_TRIANGLEDELAUNAY
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class DelaunayTriangulator : public Object{
	public:
		DLLEXPORT static vector<Float3> RunTriangulation(const vector<Float3> &inputface);

	private:
		DelaunayTriangulator::DelaunayTriangulator();
		DelaunayTriangulator::~DelaunayTriangulator();
	};

}
#endif