#include "stdafx.h"
#define _USE_MATH_DEFINES
#include <iostream>
#include<math.h>
using namespace std;
#include<vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include<math.h>

/* ------------------------------------------------------------------------
Internal includes
---------------------------------------------------------------------------*/
#include "Gmsh.h"
#include "Messages.h"
#include "ShapeFunctions.h"

GetMesh::~GetMesh() {}


GetMesh::GetMesh(string filePath)
{
	Messages messages;
	messages.logMessage("Reading Gmsh mesh");

	GetTxtData datafile(filePath);
	vector<string> data = datafile.lines;
	int rows = datafile.numLines;
	vector<vector <string>> allLinesList;

	size_t maxNumNodes = 0;
	bool checkMax = false;
	int startElements;
	int startNodes = 0;
	num3DElements = 0;


	for each (string str in data)
	{

		istringstream iss(str);
		vector<string> thisLine;
		int counter = 0;
		do
		{
			string sub;
			iss >> sub;
			thisLine.push_back(sub);
		} while (iss);
		allLinesList.push_back(thisLine);

	}

	int counter = 0;
	size_t numberLines = allLinesList.size();
	int jumpLines = 0;
	for (int counter = 0; counter < numberLines; counter++)
	{
		vector <string> thisLine = allLinesList[counter];

		//Nodes coordinates
		if (thisLine[0] == "$Nodes")
		{
			string strNumNodes = allLinesList[counter + 1][0];
			numNodes = atoi(strNumNodes.c_str());
			startNodes = counter;

		}

		//Elements nodes
		if (thisLine[0] == "$Elements")
		{
			string strNustrnumElements = allLinesList[counter + 1][0];
			numElements = atoi(strNustrnumElements.c_str());
			checkMax = true;
			startElements = counter;
		}

		size_t thisLineLen = thisLine.size();
		if (thisLineLen > maxNumNodes && checkMax)
			maxNumNodes = thisLineLen;

	}

	//cout << "Nodes coordinates" << endl;
	nodesCoordinates.resize(numNodes);
	for (int i = 0; i < numNodes; i++)
	{
		nodesCoordinates[i].resize(3);
		nodesCoordinates[i][0] = stod(allLinesList[startNodes + 2 + i][1].c_str());
		nodesCoordinates[i][1] = stod(allLinesList[startNodes + 2 + i][2].c_str());
		nodesCoordinates[i][2] = stod(allLinesList[startNodes + 2 + i][3].c_str());
		//cout << nodesCoordinates[i][0] << " , " << nodesCoordinates[i][1] << " , " << nodesCoordinates[i][2] << endl;
		jumpLines++;
	}
	elemNodes.resize(numElements);
	for (int i = 0; i < numElements; i++)
	{
		//Nodes of the elements
		vector<string> thisElement = allLinesList[i + startElements + 2];
		size_t numStrs = allLinesList[i + startElements + 2].size();
		int colCounter = 0;
		for (int j = 5; j < numStrs - 1; j++)
		{
			string thisNode = thisElement[j];
			elemNodes[i].push_back(stoi(thisNode.c_str()) - 1);
			//cout << elemNodes[i][colCounter] << ",";
			colCounter++;
		}
		numNodesPerElement.push_back(colCounter);

		//Type of the elements
		int thisElemType = stoi(thisElement[1].c_str());
		elemTypes.push_back(thisElemType);

		//Tags of the elements
		physicalTags.push_back(stoi(thisElement[3].c_str()));
		elementaryTags.push_back(stoi(thisElement[4].c_str()));

		//Number of 3-D elements
		if (thisElemType>=4)
		{
			num3DElements++;
		}
	}
	messages.logMessage("Reading Gmsh mesh: Done");
}



void GetMesh::writeMesh(GetMesh mesh, string filePath, vector<int> IDs, vector<int> VolID) {

	Messages messages;
	messages.logMessage("Writing Gmsh mesh");

	vector<vector<double>>  nodesCoordinates = mesh.nodesCoordinates;
	vector<vector<int>> elemNodes = mesh.elemNodes;
	vector<int> elemTypes = mesh.elemTypes;
	vector<int> physicalTasg = mesh.physicalTags;
	vector<int> elementaryTags = mesh.elementaryTags;
	int numNodes = mesh.numNodes;
	int numElements = mesh.numElements;

	ofstream myfile;
	myfile.open(filePath);

	myfile << "$MeshFormat\n";
	myfile << "2.2 0 8\n";
	myfile << "$EndMeshFormat\n";
	myfile << "$Nodes\n";
	myfile << numNodes << "\n";
	for (int i = 0; i < numNodes; i++)
	{
		myfile << i + 1 << " " << nodesCoordinates[i][0] << " " << nodesCoordinates[i][1] << " " << nodesCoordinates[i][2] << "\n";
	}

	size_t sizeIDs = IDs.size();
	myfile << "$EndNodes\n";
	myfile << "$Elements\n";
	vector<int> listElem;

	for (size_t i = 0; i < numElements; i++)
	{

		//Checks is this element is in the list
		if (std::find(IDs.begin(), IDs.end(), i) != IDs.end())
		{
			listElem.push_back(i);
		}
		
	}

	myfile << listElem.size() << "\n";
	int counter = 0;
	for each (int elemID in listElem)
	{
		myfile << counter + 1 << " " << elemTypes[elemID] << " " << "2" << " " << physicalTags[elemID] << " " << elementaryTags[elemID] << " ";
		for (size_t k = 0; k < elemNodes[elemID].size(); k++)
		{
			myfile << elemNodes[elemID][k] + 1;
			if (k < elemNodes[elemID].size() - 1)
			{
				myfile << " ";
			}
		}
		myfile << "\n";
		counter++;
	}
	myfile << "$EndElements\n";

	myfile.close();
	messages.logMessage("Writing Gmsh mesh: Done");

}

vector<int> GetMesh::getGaussPointsSurface(GetMesh mesh, int phySurfaceFilter, vector<int> phyVolumeFilter, vector<vector<double>> &normalVectors, vector<double> &area) {

	vector<vector<double>>  nodesCoordinates = mesh.nodesCoordinates;
	vector<vector<int>> elemNodes = mesh.elemNodes;
	vector<int> elemTypes = mesh.elemTypes;
	vector<int> physicalTasg = mesh.physicalTags;
	vector<int> elementaryTags = mesh.elementaryTags;
	vector<int> numNodesPerElem = mesh.numNodesPerElement;
	int numNodes = mesh.numNodes;
	int numElements = mesh.numElements;
	Operations oper;
	vector<int> elementsBothDomains;

	vector < vector<int>>  twoDElementNodes;
	vector<int> globalID;
	vector<vector<double>> printNormalVectorPosition;


	for (int i = 0; i < numElements; i++)
	{
		//Chek if the element is 2D
		if (elemTypes[i] < 3 && physicalTags[i] == phySurfaceFilter)
		{

			vector<int> this2DElemNodes = elemNodes[i];
			int PointID = this2DElemNodes[0];
			vector<double> P1 = { nodesCoordinates[PointID][0],nodesCoordinates[PointID][1],nodesCoordinates[PointID][2] };

			PointID = this2DElemNodes[1];
			vector<double> P2 = { nodesCoordinates[PointID][0],nodesCoordinates[PointID][1],nodesCoordinates[PointID][2] };

			PointID = this2DElemNodes[2];
			vector<double> P3 = { nodesCoordinates[PointID][0],nodesCoordinates[PointID][1],nodesCoordinates[PointID][2] };

			Vector1D thisMath;
			vector<double> A = thisMath.subtract(P2, P1);
			vector<double> B = thisMath.subtract(P3, P1);
			vector<double> AxB = thisMath.crossProduct(A, B);
			double absAxB = thisMath.Abs(AxB);
			vector<double> normal = thisMath.multiScal(AxB, 1.0 / absAxB);

			//Area of the surface element
			area.push_back(0.5*absAxB);

			int thisElemType = elemTypes[i];
			GaussLegendrePoints thisElemGauss(thisElemType);
			for (int pointCounter = 0; pointCounter < thisElemGauss.pointsCoordinates.rows; pointCounter++)
			{
				//UVP
				vector<double>pFielduv;
				pFielduv = thisElemGauss.pointsCoordinates.mat[pointCounter];

				//XYZ
				vector<double> pFieldxy = oper.scalLocalToReal(thisElemType, i, mesh, pFielduv);

				normalVectors.push_back(normal);
				printNormalVectorPosition.push_back(pFieldxy);
			}
		}
	}



	PostProcessing post;
	//string path("C:\\Anderson\\Pessoal\\01_Doutorado\\10_Testes\\34_Atuador\\03_vert - Subproblems\\02_FFEM_complete");
	//post.writeVectorField(printNormalVectorPosition, normalVectors, "NormalVector", path + "\\results\\Gmsh_Normal_Vector.txt");

	return elementsBothDomains;
}

vector<int> GetMesh::defineVolumeBoundary(GetMesh &mesh, int phySurfaceFilter, vector <int> phyVolumeFilter, int atLeastNumNodes, vector<vector<double>> &normalVectors, vector<double>  &area, string file_path) {

	vector<vector<double>>  nodesCoordinates = mesh.nodesCoordinates;
	vector<vector<int>> elemNodes = mesh.elemNodes;
	vector<int> elemTypes = mesh.elemTypes;
	vector<int> physicalTasg = mesh.physicalTags;
	vector<int> elementaryTags = mesh.elementaryTags;
	vector<int> numNodesPerElem = mesh.numNodesPerElement;
	int numNodes = mesh.numNodes;
	int numElements = mesh.numElements;
	Operations oper;
	vector<int> elementsBothDomains;

	vector < vector<int>>  twoDElementNodes;
	vector<int> globalID;
	vector<vector<double>> printNormalVectorPosition;


	for (int i = 0; i < numElements; i++)
	{

		if (elemTypes[i] < 3 && physicalTags[i] == phySurfaceFilter)
		{

			//Get the nodes of this 2D element 
			vector<int> this2DElemNodes;
			for (size_t k = 0; k < elemNodes[i].size(); k++)
			{
				this2DElemNodes.push_back(elemNodes[i][k]);
			}


			for (int elemCounter = i; elemCounter < numElements; elemCounter++)
			{

				if (elemTypes[elemCounter] >= 4 && std::find(phyVolumeFilter.begin(), phyVolumeFilter.end(), physicalTags[elemCounter]) != phyVolumeFilter.end())
				{
					vector<int> this3DElemNodes;

					//Get the nodes of this 3D element
					int numNodes3D = numNodesPerElem[elemCounter];
					for (int k = 0; k < numNodes3D; k++)
					{
						this3DElemNodes.push_back(elemNodes[elemCounter][k]);
					}

					//Verify if the 3D element contains all the nodes of the 2D element
					bool addThisElem = false;
					size_t numNodes2D = this2DElemNodes.size();
					int foundCounter = 0;
					for (int DElemCounter = 0; DElemCounter < numNodes2D; DElemCounter++)
					{
						if (std::find(this3DElemNodes.begin(), this3DElemNodes.end(), this2DElemNodes[DElemCounter]) != this3DElemNodes.end()) {
							foundCounter++;
						}

						if (foundCounter == atLeastNumNodes)
						{
							addThisElem = true;
							break;
						}
					}

					if (addThisElem)
					{
						//Gets this element ID
						if (std::find(elementsBothDomains.begin(), elementsBothDomains.end(), elemCounter) != elementsBothDomains.end())
						{
						}
						else {
							elementsBothDomains.push_back(elemCounter);
						}
						twoDElementNodes.push_back(this2DElemNodes);
						globalID.push_back(i);
						//break;
					}
				}
			}
		}
	}

	//Computes the normal vector
	int counter = 0;
	for each (int elemID in elementsBothDomains)
	{
		vector<int> this2DElemNodes = twoDElementNodes[counter];
		int PointID = this2DElemNodes[0];
		vector<double> P1 = { nodesCoordinates[PointID][0],nodesCoordinates[PointID][1],nodesCoordinates[PointID][2] };

		PointID = this2DElemNodes[1];
		vector<double> P2 = { nodesCoordinates[PointID][0],nodesCoordinates[PointID][1],nodesCoordinates[PointID][2] };

		PointID = this2DElemNodes[2];
		vector<double> P3 = { nodesCoordinates[PointID][0],nodesCoordinates[PointID][1],nodesCoordinates[PointID][2] };

		Vector1D thisMath;
		vector<double> A = thisMath.subtract(P2, P1);
		vector<double> B = thisMath.subtract(P3, P1);
		vector<double> AxB = thisMath.crossProduct(A, B);
		double absAxB = thisMath.Abs(AxB);
		vector<double> normal = thisMath.multiScal(AxB, 1.0 / absAxB);

		//Area of the surface element
		area.push_back(0.5*absAxB);

		int thisElemType = elemTypes[elemID];
		GaussLegendrePoints thisElemGauss(thisElemType);
		for (int pointCounter = 0; pointCounter < thisElemGauss.pointsCoordinates.rows; pointCounter++)
		{
			//UVP
			vector<double>pFielduv;
			pFielduv = thisElemGauss.pointsCoordinates.mat[pointCounter];

			//XYZ
			vector<double> pFieldxy = oper.scalLocalToReal(thisElemType, elemID, mesh, pFielduv);

			normalVectors.push_back(normal);
			printNormalVectorPosition.push_back(pFieldxy);
		}

		counter++;
	}

	PostProcessing post;
	post.writeVectorField(printNormalVectorPosition, normalVectors, "NormalVector", file_path + "results\\Gmsh_Normal_Vector.txt");

	return elementsBothDomains;
}


GetTxtData::~GetTxtData(void) {}
GetTxtData::GetTxtData(string filePath) {
	string line;
	ifstream myfile(filePath);
	Messages messages;
	
	numLines = 0;
	if (myfile.is_open())
	{
		while (getline(myfile, line))
		{
			lines.push_back(line);
			numLines++;
		}
		myfile.close();
	}
	else messages.logMessage("Unable to open this file:" + filePath);;
}

void PostProcessing::writeVectorField(vector<vector<double>> coordinates, vector<vector<double>> fields, string fieldName, string path) {

	Messages messages;
	messages.logMessage("Writing Gmsh vector field " + fieldName);

	ofstream myfile;
	myfile.open(path);
	myfile << "View \"" << fieldName << "\" { \n";
	for (size_t i = 0; i < coordinates.size(); i++)
	{
		myfile << "VP(" << coordinates[i][0] << "," << coordinates[i][1] << "," << coordinates[i][2] << ")";
		myfile << "{" << fields[i][0] << "," << fields[i][1] << "," << fields[i][2] << "};\n";
	}

	myfile << "TIME{ 1 };\n};";
	myfile.close();
	messages.logMessage("Writing Gmsh vector field " + fieldName + ":Done");


}

void PostProcessing::writeGaussPointsIDs(vector<int>elemIDs, vector<vector<int>> pointsIDPerElement, vector<vector<double>> pointsCoordinates, string path)
{
	Messages messages;
	messages.logMessage("Writing Gauss points");

	string fileNameID("results//Gauss_Points_IDs.txt");
	string fileNameCoord("results//Gauss_Points_Coordinates.txt");
	ofstream myfile;
	string sep = " ";

	/* ------------------------------------------------------------------------
	Writes the IDS
	---------------------------------------------------------------------------*/
	myfile.open(path + fileNameID);
	int elemCounter = 0;
	size_t sizeElemIDs = elemIDs.size();
	for each (vector<int> thisElemPoints in pointsIDPerElement)
	{
		if (sizeElemIDs > 0)
			myfile << elemIDs[elemCounter] << sep;
		else
			myfile << elemCounter << sep;

		size_t counter = 0;
		for each (int thisID in thisElemPoints)
		{
			myfile << thisID;
			if (counter < thisElemPoints.size() - 1)
			{
				myfile << sep;
			}
			counter++;
		}
		myfile << "\n";
		elemCounter++;
	}
	myfile.close();

	/* ------------------------------------------------------------------------
	Writes the coordinates
	---------------------------------------------------------------------------*/
	myfile.open(path + fileNameCoord);

	for each (vector<double> point in pointsCoordinates)
	{
			myfile << point[0] << " " << point[1] << " " << point[2] << "\n";
	}

	myfile.close();

	messages.logMessage("Writing Gauss points: Done");
}

void PostProcessing::writeDataResults(vector<vector<double>> twoDArrayData, string path, string fileName)
{
	Messages messages;
	messages.logMessage("Writing data file " + fileName);

	string filePath(path + "results//" + fileName + ".txt");
	ofstream myfile;
	myfile.open(filePath);
	for each (vector<double> thisRow in twoDArrayData)
	{
		size_t counter = 0;
		for each (double thisData in thisRow)
		{
			myfile << thisData;
			if (counter < thisRow.size() - 1)
			{
				myfile << " ";
			}
			counter++;
		}
		myfile << "\n";
	}
	myfile.close();

	messages.logMessage("Writing data file " + fileName + ": Done");

}

void PostProcessing::getFieldComponents(vector<vector<double>> &normal, vector<vector<double>> &tangent, vector<double> area, vector<vector<double>> field, vector<vector<double>> pointsCoordinates, vector<int>elemIDs, vector<vector<int>> pointsIDPerElement, string fileName, string path, GetMesh mesh)
{
	vector<vector<double>>  nodesCoordinates = mesh.nodesCoordinates;
	vector<vector<int>> elemNodes = mesh.elemNodes;
	vector<int> elemTypes = mesh.elemTypes;
	vector<int> physicalTasg = mesh.physicalTags;
	vector<int> elementaryTags = mesh.elementaryTags;
	vector<int> numNodesPerElem = mesh.numNodesPerElement;
	int numNodes = mesh.numNodes;
	int numElements = mesh.numElements;
	Operations oper;
	vector<vector<double>> flux;

	vector<vector<double>> unitaryNormal;
	vector<vector<double>> unitaryNormalPosition;

	for (int i = 0; i < elemIDs.size(); i++)
	{
		int thisElemID = elemIDs[i];
		int thisElemType = elemTypes[thisElemID];

		vector<int> thisElemNodes = elemNodes[thisElemID];
		double this_ElemFlux = 0; this_ElemFlux;
		if (elemTypes[i] < 3)
		{
			//Gets the nodes of the surface element
			int PointID = thisElemNodes[0];
			vector<double> P1 = { nodesCoordinates[PointID][0],nodesCoordinates[PointID][1],nodesCoordinates[PointID][2] };
			PointID = thisElemNodes[1];
			vector<double> P2 = { nodesCoordinates[PointID][0],nodesCoordinates[PointID][1],nodesCoordinates[PointID][2] };
			PointID = thisElemNodes[2];
			vector<double> P3 = { nodesCoordinates[PointID][0],nodesCoordinates[PointID][1],nodesCoordinates[PointID][2] };

			Vector1D thisMath;
			vector<double> A = thisMath.subtract(P2, P1);
			vector<double> B = thisMath.subtract(P3, P1);
			vector<double> AxB = thisMath.crossProduct(A, B);
			double absAxB = thisMath.Abs(AxB);
			vector<double> unitNormal = thisMath.multiScal(AxB, 1.0 / absAxB);

			//Area of the surface element
			area.push_back(0.5*absAxB);

			vector <int> thisElemPointsID = pointsIDPerElement[i];

			GaussLegendrePoints thisElemGauss(thisElemType);
			int numGaussPoints = thisElemGauss.pointsCoordinates.rows;
			double sumBn = 0;
			for (int pointCounter = 0; pointCounter < numGaussPoints; pointCounter++)
			{

				//UVP
				vector<double>pFielduv;
				pFielduv = thisElemGauss.pointsCoordinates.mat[pointCounter];

				//XYZ
				vector<double> pFieldxy = oper.scalLocalToReal(thisElemType, thisElemID, mesh, pFielduv);

				unitaryNormal.push_back(unitNormal);
				unitaryNormalPosition.push_back(pFieldxy);

				//Field at this point
				vector<double> fieldThisPoint = field[thisElemPointsID[pointCounter]];
				vector<double> thisTangent = thisMath.multiScal(thisMath.crossProduct(unitNormal, thisMath.crossProduct(unitNormal, fieldThisPoint)), -1.0);
				vector<double> thisNormal = thisMath.subtract(fieldThisPoint, thisTangent);

				tangent.push_back(thisTangent);
				normal.push_back(thisNormal);


				//Check the direction of the fields
				double k_direction = 1;
				double r = thisMath.dot(thisNormal, unitNormal);
				if (r < 0.0)
					k_direction = -1.0;

				//sumBn
				double Bn = k_direction * 4 * M_PI*pow(10, -7)*thisMath.Abs(thisNormal);
				sumBn += Bn;
			}
			double thisFlux = sumBn*0.5*absAxB / numGaussPoints;
			vector<double> thisData = { double(thisElemID),thisFlux };
			flux.push_back(thisData);
		}
	}


	PostProcessing post;
	post.writeVectorField(pointsCoordinates, tangent, "Ht", path + "\\results\\" + fileName + "_tangent.txt");
	post.writeVectorField(pointsCoordinates, normal, "Hn", path + "\\results\\" + fileName + "_normal.txt");
	post.writeVectorField(unitaryNormalPosition, unitaryNormal, "Normal unitary", path + "\\results\\" + fileName + "_normal_nitary.txt");

	post.writeDataResults(flux, path, "flux_surface");

}

vector<vector<double>> PostProcessing::readDataFile(string path, string fileName)
{

	Messages messages;
	messages.logMessage("Reading data file");

	string filePath(path + "results//" + fileName + ".txt");
	GetTxtData datafile(filePath);
	vector<string> data = datafile.lines;
	int rows = datafile.numLines;
	vector<vector <double>> allLinesList;

	for each (string str in data)
	{
		istringstream iss(str);
		vector<double> thisLine;
		int counter = 0;
		do
		{
			string sub;
			iss >> sub;
			if (sub != "")
			{
				thisLine.push_back(stod(sub));

			}
		} while (iss);
		allLinesList.push_back(thisLine);
	}

	return allLinesList;
}
