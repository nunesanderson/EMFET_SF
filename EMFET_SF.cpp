#include "stdafx.h"
#include <iostream>
#include<math.h>
using namespace std;
#include<vector>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>
/* ------------------------------------------------------------------------
Internal includes
---------------------------------------------------------------------------*/
#include "Biot-Savart.h"
#include "ShapeFunctions.h"
#include "Matrix.h"
#include "Gmsh.h"
#include "Messages.h"
#include "Tests-Biot-Savart.h"
#include <ctime>


int main()
{
	Messages messages;

	TestsBiotSavart testBS;
	double fw = 1.0;
	double elapsed_secs = 0;
	testBS.Atuador3D();
	messages.logMessage("Finished!");
	getchar();  
	return 0;
}
