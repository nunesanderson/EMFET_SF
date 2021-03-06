#include "stdafx.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "AnalyticalSolutions.h"

vector<double> BiotSavartAnalyt::getHcoil(double radius, vector<double> listZPos,double len, double current, double turns)
{
	vector<double>ans;

	for each (double z in listZPos)
	{
		double n = turns / len;
		double a1 = (len / 2.0 - z) / sqrt(pow(z - len / 2.0, 2) + pow(radius, 2.0));
		double a2 = (len / 2.0 + z) / sqrt(pow(z + len / 2.0, 2) + pow(radius, 2.0));

		ans.push_back(current*n/2.0*(a1+a2));

	}
	return ans;
}

vector<double> BiotSavartAnalyt::getHCurrentLoop(double radius, vector<double> listZPos, double current)
{
	vector<double>ans;
	for each (double z in listZPos)
	{
		double k = 2*pow(pow(radius , 2) + pow(z, 2.0),3.0/2.0);
		ans.push_back(current*pow(radius,2) / k);
	}
	return ans;
}
