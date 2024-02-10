#include<cstdio>
#include<cstring>

unsigned short getNElemNodes(unsigned short &ElemType)
{
	unsigned short vtkElemType;
	unsigned short nElemNodes;
	switch(ElemType){
		case 1: nElemNodes = 8; vtkElemType = 12; break;
		case 2: nElemNodes = 6; vtkElemType = 13; break;
		case 3: nElemNodes = 4; vtkElemType = 10; break;
		case 4: nElemNodes = 20; vtkElemType = 25; break;
		case 6: nElemNodes = 10; vtkElemType = 24; break;
	}

	ElemType = vtkElemType;
	return nElemNodes;
}


struct vtkHandle;
struct simEigHandle;

struct frdHandle{
	unsigned short elmType;
	unsigned long nNodes;
	unsigned long nElems;
	unsigned long nSteps;

	FILE *frdFile;
	vtkHandle **vtkHd;
	simEigHandle **simEigHd;

	frdHandle(char frdProb[50], unsigned long inSteps);
	~frdHandle();
	void readCrd();
	void readCnn();
	void readfrqData();
	void TransferData(char analysisType[50]);
};

struct simEigHandle{
	FILE *simEigFile;

	simEigHandle(char simEigFileName[50]);
	~simEigHandle();
	inline void writeEigFrq(unsigned long modeidx, double freq){fprintf(simEigFile,"%lu %lf\n",modeidx,freq);}
	inline void writeVector(unsigned long nodeidx, double *vec){fprintf(simEigFile,"%lu %lf %lf %lf\n",nodeidx,vec[0],vec[1],vec[2]);}

};

simEigHandle::simEigHandle(char simEigFileName[50])
{
	simEigFile = fopen(simEigFileName,"w");
}

simEigHandle::~simEigHandle()
{
	if(simEigFile)
		fclose(simEigFile);
	simEigFile = NULL;
}

struct vtkHandle{
	FILE *vtkFile;

	vtkHandle(char vtkFileName[50]);	
	~vtkHandle();
	inline void writeNPts(unsigned long nPts) {fprintf(vtkFile,"POINTS %lu FLOAT\n",nPts);}
	inline void writeCrd(double x, double y, double z) {fprintf(vtkFile,"%lf %lf %lf\n",x,y,z);}
	inline void writeNElems(unsigned long nElems, unsigned long nNodes) {fprintf(vtkFile,"CELLS %lu %lu\n",nElems, nNodes);}
	inline void writeNCellType(unsigned long nElems){fprintf(vtkFile,"CELL_TYPES %lu\n",nElems);}
	inline void writeCellType(unsigned short cellType){fprintf(vtkFile,"%hu\n",cellType);}
	inline void writePointData(unsigned long nNodes){fprintf(vtkFile,"POINT_DATA %lu\n",nNodes);}
	inline void writeInitVector(){fprintf(vtkFile,"VECTORS U FLOAT\n");}
	inline void writeVector(double *vec){fprintf(vtkFile,"%lf %lf %lf\n",vec[0],vec[1],vec[2]);}
	void writeCnn(unsigned long *elemCnn, unsigned short nElemNodes);
	void writeData();
};

vtkHandle::vtkHandle(char vtkFileName[50])
{
	vtkFile = fopen(vtkFileName,"w");
	fprintf(vtkFile,"# vtk DataFile Version 2.0\n");
	fprintf(vtkFile,"TITLE = \"VTK OUTPUT\"\n");
	fprintf(vtkFile,"ASCII\n");
	fprintf(vtkFile,"DATASET UNSTRUCTURED_GRID\n");
}

vtkHandle::~vtkHandle()
{
	if(vtkFile != NULL)
		fclose(vtkFile);
	vtkFile = NULL;
}

void vtkHandle::writeCnn(unsigned long *elemCnn, unsigned short nElemNodes)
{
	fprintf(vtkFile,"%hu ",nElemNodes);
	for(unsigned long iNode = 0; iNode < nElemNodes; iNode++)
		fprintf(vtkFile,"%lu ",elemCnn[iNode]);
	fprintf(vtkFile,"\n");
}

frdHandle::frdHandle(char frdProb[50], unsigned long inSteps): nSteps(inSteps)
{
	//open the frd file
	char frdFileName[55];
	sprintf(frdFileName,"%s.frd",frdProb);
	frdFile = fopen(frdFileName,"r");

	//Allocate the vtk File Handles and open the vtkFiles
	vtkHd = new vtkHandle*[nSteps];
	char vtkFileName[75];
	for(unsigned long i = 0; i < nSteps; i++)
	{
		sprintf(vtkFileName,"%s.%lu.vtk",frdProb,i+1);
		vtkHd[i] = new vtkHandle(vtkFileName);
	}

	//Allocate the simEig Handles and open the simEig files
	simEigHd = new simEigHandle*[nSteps];
	for(unsigned long i = 0; i < nSteps; i++)
	{
		sprintf(vtkFileName,"%s.%lu.raw",frdProb,i+1);
		simEigHd[i] = new simEigHandle(vtkFileName);
	}
}

frdHandle::~frdHandle()
{
	//Close the simEig Files and Deallocate the memory
	for(unsigned long i = 0; i < nSteps; i++){
		delete simEigHd[i];
		simEigHd[i] = NULL;
	}
	delete[] simEigHd;
	simEigHd = NULL;

	//Close the vtk Files and deallocate the memory
	for(unsigned long i = 0; i < nSteps; i++){
		delete vtkHd[i];
		vtkHd[i] = NULL;
	}
	delete[] vtkHd;
	vtkHd = NULL;

	//close the frd File
	fclose(frdFile);
	frdFile = NULL;
}

void frdHandle::readCrd()
{
	//Locate the co-ordinates
	char Key[100];
	int format;
	while(1){
		fscanf(frdFile,"%s",Key);
		if(!strcmp(Key,"2C")){
			fscanf(frdFile,"%lu %d",&nNodes,&format);
			printf("nNodes = %lu\n", nNodes);
			for(unsigned long fileidx = 0; fileidx < nSteps; fileidx++)
				vtkHd[fileidx]->writeNPts(nNodes);
			break;
		}
	}

	//Read Nodes
	printf("Reading and writing Nodal Co-ordinates.\n");
	double x, y, z;
	unsigned long nodeidx;
	int key;
	for(unsigned long i = 0; i < nNodes; i++)
	{
		fscanf(frdFile,"%d %lu %lf %lf %lf",&key, &nodeidx, &x, &y, &z);
		for(unsigned long fileidx = 0; fileidx < nSteps; fileidx++)
				vtkHd[fileidx]->writeCrd(x,y,z);
	}
	printf("Nodal Co-ordinates written to vtk Files.\n");
}

void frdHandle::readCnn()
{
	//Locate the connectivity
	char Key[100];
	int format;
	while(1){
		fscanf(frdFile,"%s",Key);
		if(!strcmp(Key,"3C")){
			fscanf(frdFile,"%lu %d",&nElems,&format);
			printf("nElems = %lu\n", nElems);
			break;
		}
	}

	//Allocate memory for holding element type and element connectivity
	unsigned short *elemTypes = new unsigned short[nElems];
	unsigned short *elemctNodes = new unsigned short[nElems];
	unsigned long **elemConn = new unsigned long*[nElems];
	int key; unsigned long elemidx; unsigned short elemType; unsigned short group; unsigned short material;
	unsigned long iElem = 0;
	unsigned short nElemNodes;
	unsigned short iElemNode;
	unsigned long elemSum = 0; unsigned short maxEntries = 10;
	while(1)
	{
		fscanf(frdFile,"%d %lu %hu %hu %hu",&key, &elemidx, &elemType, &group, &material);
		nElemNodes = getNElemNodes(elemType);
		elemSum += (nElemNodes+1);
		elemTypes[iElem] = elemType;
		elemctNodes[iElem] = nElemNodes;
		elemConn[iElem] = new unsigned long[nElemNodes];

		if(key == -1) fscanf(frdFile,"%d",&key);
		iElemNode = 0;
		for(unsigned short iEntry = 0; iEntry < maxEntries; iEntry++){
			if(iElemNode == nElemNodes)
				break;
			fscanf(frdFile,"%lu",&elemConn[iElem][iElemNode]);
			elemConn[iElem][iElemNode]-=1;
			iElemNode++;
		}

		if(iElemNode != nElemNodes){
			fscanf(frdFile,"%d",&key);
			// printf("key = %d",key);
			for(unsigned short iEntry = 0; iEntry < maxEntries; iEntry++){
				fscanf(frdFile,"%lu",&elemConn[iElem][iElemNode]);
				elemConn[iElem][iElemNode]-=1;
				iElemNode++;
			}
		}


		iElem++;
		if(iElem == nElems)
			break;
	}



	for(unsigned long fileidx = 0; fileidx < nSteps; fileidx++){
		vtkHd[fileidx]->writeNElems(nElems, elemSum);
		for(iElem = 0; iElem < nElems; iElem++)
			vtkHd[fileidx]->writeCnn(elemConn[iElem], elemctNodes[iElem]);
		vtkHd[fileidx]->writeNCellType(nElems);
		for(iElem = 0; iElem < nElems; iElem++)
			vtkHd[fileidx]->writeCellType(elemTypes[iElem]);
	}
		
	printf("Connectivity Data written to vtk files.\n");
	//DeAllocate memory 
	for(iElem = 0; iElem < nElems; iElem++){
		delete[] elemConn[iElem];
		elemConn[iElem] = NULL;
	}
	delete[] elemConn;
	elemConn = NULL;

	delete[] elemTypes;
	elemTypes = NULL;

	delete[] elemctNodes;
	elemctNodes = NULL;
}

void frdHandle::readfrqData()
{
	//Locate the modeindex
	char Key[100];
	unsigned long modeidx;

	double **nodalData = new double*[nNodes];
	for(unsigned long iNode = 0; iNode < nNodes; iNode++)
		nodalData[iNode] = new double[3];

	double eigenFrequency;
	char tmpstr[20];
	unsigned long tmpval,tmp1,tmp2;
	double tmpdat;

	unsigned long iStep = 0;
	while(iStep < nSteps){
		while(1){
			fscanf(frdFile,"%s",Key);
			if(!strcmp(Key,"1PSTEP")){
				fscanf(frdFile,"%lu %lu %lu",&modeidx, &tmp1, &tmp2);
				printf("modeidx = %lu\n", modeidx);
				break;
			}
		}

		//Get the Modal Mass
		fscanf(frdFile,"%s %lf",tmpstr,&tmpdat);
		printf("Modal mass = %lf\n",tmpdat);
		//Get the eigenFrequency
		fscanf(frdFile,"%s %lf",tmpstr,&eigenFrequency);
		printf("Modal Stiffness = %lf\n",eigenFrequency);
		simEigHd[modeidx-1]->writeEigFrq(modeidx,eigenFrequency);

		vtkHd[modeidx-1]->writePointData(nNodes);
		vtkHd[modeidx-1]->writeInitVector();

		while(1){
			fscanf(frdFile,"%s",Key);
			if(!strcmp(Key,"1ALL")){
				break;
			}
		}

		int key; unsigned long nodeidx; double v1, v2, v3;
		for(unsigned long iNode = 0; iNode < nNodes; iNode++){
			fscanf(frdFile,"%d %lu %lf %lf %lf",&key, &nodeidx, &v1, &v2, &v3);

			nodalData[nodeidx-1][0] = v1;
			nodalData[nodeidx-1][1] = v2;
			nodalData[nodeidx-1][2] = v3;
		}

		for(unsigned long iNode = 0; iNode < nNodes; iNode++){
			vtkHd[modeidx-1]->writeVector(nodalData[iNode]);
			simEigHd[modeidx-1]->writeVector(iNode+1,nodalData[iNode]);
		}
		iStep++;

		printf("modal data for %lu mode written.\n",modeidx);
	}







	for(unsigned long iNode = 0; iNode < nNodes; iNode++){
		delete[] nodalData[iNode];
		nodalData[iNode] = NULL;
	}
	delete[] nodalData;
	nodalData = NULL;

}



void frdHandle::TransferData(char analysisType[50])
{
	readCrd();
	readCnn();

	if(!strcmp(analysisType,"frq")){
		readfrqData();
	}
}


int main(int argc, char* argv[])
{
	unsigned long nVtkFiles;
	sscanf(argv[2],"%lu",&nVtkFiles);
	frdHandle *frdHd = new frdHandle(argv[1],nVtkFiles);
	frdHd->TransferData(argv[3]);

	delete frdHd;
	frdHd = NULL;
}
