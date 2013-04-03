#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_COMMON_MATH
#include "CommonMath.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

float MMath::RandomNumber(float Min, float Max)
{
	return ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
}
long double MMath::CoordinateDistance(POINT pos1,POINT pos2){
	long double pos1X=pos1.x;
	long double pos1Y=pos1.y;
	long double pos2X=pos2.x;
	long double pos2Y=pos2.y;
	long double Distance=0;
	//int xdist=pos1X-pos2X;
	//int ydist=pos1Y-pos2Y;
	Distance=sqrt( (pos2X - pos1X)*(pos2X - pos1X) + (pos2Y - pos1Y)*(pos2Y - pos1Y));


	return Distance;
}
float MMath::CoordinateDistance(float x1, float x2,float y1, float y2){
	float Distance=0;
	//int xdist=pos1X-pos2X;
	//int ydist=pos1Y-pos2Y;
	Distance=sqrt( (x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));


	return Distance;



}
double MMath::AngleBetweenPoints(POINT pos1, POINT pos2){
	double angle = 0;

	angle = atan2((float)pos2.y - (float)pos1.y, (float)pos2.x - (float)pos1.x);


	return angle;
}
double MMath::AngleBetweenPoints(float x1, float x2,float y1, float y2){
	double angle = 0;

	angle = atan2(y2 - y1, x2 - x1);


	return angle;


}
POINT MMath::GetRelativeMousePos(HWND Hwnd){
	POINT MousePos;
	GetCursorPos(&MousePos);
	ScreenToClient(Hwnd,&MousePos);

	return MousePos;
}

int MMath::GreatestCommonDivisor(int a, int b){
	return (b == 0 ? a : GreatestCommonDivisor(b, a%b));
}

DLLEXPORT  bool Leviathan::MMath::IsPointInsidePolygon(const vector<Float3>& polygon, const Float3& point){
	//bool IsInside = false;
	//int i,j;
	//for(i = 0, j = polygon.size()-1; i < polygon.size(); j = i++){
	//	if(((polygon[i].Val[1] > point.Val[1]) != (polygon[j].Val[1] > point.Val[1])) 
	//		&& (point.Val[0] < (polygon[j].Val[0]-polygon[i].Val[0]) * (point.Val[1]-polygon[i].Val[1]) / (polygon[j].Val[1]-polygon[i].Val[1]) + polygon[i].Val[0]))
	//		IsInside = !IsInside;
	//}
	//return IsInside;

	// better? one http://devmaster.net/forums/topic/5213-point-in-polygon-test/
	Float3 p = (polygon[polygon.size()-1] - point).Cross(polygon[0] - point);
	for (unsigned int i = 0; i < polygon.size() - 1; i++){

		Float3 q = (polygon[i] - point).Cross(polygon[i+1] - point);
		if(p.Dot(q) < 0)
			return false;
	}
	return true;

}

DLLEXPORT bool Leviathan::MMath::IsEqual(double x, double y){
	const double epsilon = 1e-5;
	return abs(x - y) <= epsilon * abs(x);
}

DLLEXPORT bool Leviathan::MMath::IsEqual(float x, float y){
	const double epsilon = 1e-5;
	return abs(x - y) <= epsilon * abs(x);
}

