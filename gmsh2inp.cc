#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <unistd.h>


struct VolCnnMap{
	unsigned short nElemNodes;
	bool ReducedFlag;
	char elemName[20];
	unsigned short connectivityMap[20];

	VolCnnMap(unsigned short nen, bool rf): nElemNodes(nen), ReducedFlag(rf)
	{
		switch(nElemNodes){
			case 4 : sprintf(elemName,"C3D4"); 
				 connectivityMap[0] = 0; connectivityMap[1] = 1;
				 connectivityMap[2] = 2; connectivityMap[3] = 3;
				 break;
			case 10: sprintf(elemName,"C3D10");
				 connectivityMap[0] = 0; connectivityMap[1] = 1;
				 connectivityMap[2] = 2; connectivityMap[3] = 3;
				 connectivityMap[4] = 4; connectivityMap[5] = 5;
				 connectivityMap[6] = 6; connectivityMap[7] = 7;
				 connectivityMap[8] = 9; connectivityMap[9] = 8;
				 break;
			case 6 : sprintf(elemName,"C3D6"); 
				 connectivityMap[0] = 0; connectivityMap[1] = 1;
				 connectivityMap[2] = 2; connectivityMap[3] = 3;
				 connectivityMap[4] = 4; connectivityMap[5] = 5;
				 break;
			case 15: sprintf(elemName,"C3D15");
				 connectivityMap[0] = 0; connectivityMap[1] = 1;
				 connectivityMap[2] = 2; connectivityMap[3] = 3;
				 connectivityMap[4] = 4; connectivityMap[5] = 5;
				 connectivityMap[6] = 6; connectivityMap[7] = 8;
				 connectivityMap[8] = 7; connectivityMap[9] = 12;
				 connectivityMap[10] = 14; connectivityMap[11] = 13;
				 connectivityMap[12] = 8; connectivityMap[13] = 10;
				 connectivityMap[14] = 11;
				 break;
			case 8 : sprintf(elemName,"C3D8");
				 connectivityMap[0] = 0; connectivityMap[1] = 1;
				 connectivityMap[2] = 2; connectivityMap[3] = 3;
				 connectivityMap[4] = 4; connectivityMap[5] = 5;
				 connectivityMap[6] = 6; connectivityMap[7] = 7;
				 break;
			case 20: sprintf(elemName,"C3D20"); 
				 connectivityMap[0] = 0; connectivityMap[1] = 1; 
				 connectivityMap[2] = 2; connectivityMap[3] = 3;
				 connectivityMap[4] = 4; connectivityMap[5] = 5;
				 connectivityMap[6] = 6; connectivityMap[7] = 7;
				 connectivityMap[8] = 8; connectivityMap[9] = 11;
				 connectivityMap[10] = 13; connectivityMap[11] = 9;
				 connectivityMap[12] = 16; connectivityMap[13] = 18;
				 connectivityMap[14] = 19; connectivityMap[15] = 17;
				 connectivityMap[16] = 10; connectivityMap[17] = 12;
				 connectivityMap[18] = 14; connectivityMap[19] = 15;
				 break;
		}

		if(ReducedFlag)
			sprintf(elemName,"%sR",elemName);
	}
};

struct Config{
	char* problemName;
	char* inpFileName;
	char* cnnfileName;
	bool reducedFlag;
	unsigned short nElemNodes;
	unsigned short nSurfNodes;
	unsigned int nNodesets;
	char** nodeFileList;
	unsigned int nElemsets;
	char** elemFileList;	
	Config();
	~Config();
};

Config::Config(){
	problemName = new char[50];
	cnnfileName = new char[50];
	inpFileName = new char[50];
	nNodesets = 0; nodeFileList = NULL;
	nElemsets = 0; elemFileList = NULL;
}

Config::~Config(){
	delete[] problemName;
	delete[] cnnfileName;

	for(unsigned int i = 0; i < nNodesets; i++){
		delete[] nodeFileList[i];
		nodeFileList[i] = NULL;
	}
	delete[] nodeFileList;
	nodeFileList = NULL;

	for(unsigned int i = 0; i < nElemsets; i++){
		delete[] elemFileList[i];
		elemFileList[i] = NULL;
	}
	delete[] elemFileList;
	elemFileList = NULL;
}

void readConfig( Config* configHd )
{
	FILE *fileConfig;
	char* configFileName = "gmsh2inp.config";
	fileConfig = (access(configFileName,F_OK) == 0 ? fopen(configFileName,"r"): NULL);

	char isProblem[10];
	fscanf(fileConfig,"%s %s",isProblem, configHd->problemName);
	if(strcmp(isProblem,"PROBLEM")){
		printf("PROBLEM name not found. Exiting\n");
		exit(1);
	}

	sprintf(configHd->inpFileName,"%s.inp",configHd->problemName);

	// printf("%s\n",configHd->problemName);
	fscanf(fileConfig,"%s %s",isProblem,configHd->cnnfileName);
	if(strcmp(isProblem,"CNNFILE")){
		printf("CNNFILE name not found. Exiting\n");
		exit(1);
	}
	
	fscanf(fileConfig,"%s %u",isProblem,&(configHd->reducedFlag));
	if(strcmp(isProblem,"REDUCED")){
		printf("REDUCED name not found. Exiting\n");
		exit(1);
	}
//	printf("%s",isProblem);

	// printf("%s\n",configHd->cnnfileName);
	fscanf(fileConfig,"%s %u",isProblem,&(configHd->nElemNodes));
//	printf("%s",isProblem);
//	printf("%u\n",configHd->nElemNodes);
	if(strcmp(isProblem,"NELMNODES")){
		printf("NELMNODES name not found. Exiting\n");
		exit(1);
	}	

	
	fscanf(fileConfig,"%s %u",isProblem,&(configHd->nSurfNodes));
//	printf("%u\n",configHd->nSurfNodes);
	if(strcmp(isProblem,"NSRFNODES")){
		printf("NSRFNODES name not found. Exiting\n");
		exit(1);
	}	

	fscanf(fileConfig,"%s %u",isProblem,&(configHd->nNodesets));
//	printf("%u\n",configHd->nNodesets);
	if(strcmp(isProblem,"NNODES")){
		printf("NNODES name not found. Exiting\n");
		exit(1);
	}

	if(configHd->nNodesets > 0){
		configHd->nodeFileList = new char*[configHd->nNodesets];
		for(unsigned int i = 0; i < configHd->nNodesets; i++){
			configHd->nodeFileList[i] = new char[100];
			fscanf(fileConfig,"%s %s", isProblem, configHd->nodeFileList[i]);
			printf("%s %s\n",isProblem,configHd->nodeFileList[i]);
			if(strcmp(isProblem,"NODES")){
				printf("NODES name not found. Exiting\n");
				exit(1);
			}
		}
	}

	fscanf(fileConfig,"%s %u",isProblem,&(configHd->nElemsets));
	if(strcmp(isProblem,"NELEMS")){
		printf("NELEMS name not found. Exiting\n");
		exit(1);
	}

	if(configHd->nElemsets > 0){
		configHd->elemFileList = new char*[configHd->nElemsets];
		for(unsigned int i = 0; i < configHd->nElemsets; i++){
			configHd->elemFileList[i] = new char[100];
			fscanf(fileConfig,"%s %s", isProblem,configHd->elemFileList[i]);
			if(strcmp(isProblem,"ELEMS")){
				printf("ELEMS name not found. Exiting\n");
				exit(1);
			}
		}
	}
	
	printf("Config File read\n");
	fclose(fileConfig);
}

void writeHeading(Config* configHd)
{
	FILE* inpFile;
	inpFile = fopen(configHd->inpFileName,"w");

	fprintf(inpFile,"*HEADING\n");
	fprintf(inpFile,"Model: %s\n",configHd->problemName);

	fclose(inpFile);
}

void writeCrd(Config* configHd)
{
	char crdFileName[100];
	sprintf(crdFileName,"%s.crd",configHd->problemName);
	FILE *crdFile = (access(crdFileName,F_OK) == 0 ? fopen(crdFileName,"r"): NULL);

	FILE *inpFile = fopen(configHd->inpFileName,"a");
	fprintf(inpFile,"*NODE\n");

	unsigned long idxNode;
	double xNode, yNode, zNode;
	do 
	{
		fscanf(crdFile,"%lu %lf %lf %lf",&idxNode, &xNode, &yNode, &zNode);
		if(feof(crdFile))
			break;
		fprintf(inpFile,"%lu,%lf,%lf,%lf\n",idxNode,xNode,yNode,zNode);
	}while(!feof(crdFile));
	printf("Co-ordinates written\n");
	fclose(inpFile);
}

void writeCnn(Config* configHd)
{
	char cnnFileName[100];
	sprintf(cnnFileName,"%s.%s.cnn",configHd->problemName, configHd->cnnfileName);
	FILE *cnnFile = (access(cnnFileName,F_OK) == 0 ? fopen(cnnFileName,"r"): NULL);
	
	VolCnnMap cMap(configHd->nElemNodes, configHd->reducedFlag);

	FILE *inpFile = fopen(configHd->inpFileName,"a");
	fprintf(inpFile,"*ELEMENT, TYPE=%s, ELSET=%s",cMap.elemName,configHd->cnnfileName);

	unsigned long elemNodes[configHd->nElemNodes];
	unsigned long elemidx;
	do
	{	
		fscanf(cnnFile,"%lu",&elemidx);
		for(unsigned short i = 0; i < configHd->nElemNodes; i++)
			fscanf(cnnFile,"%lu",&elemNodes[i]);

		if(feof(cnnFile))
			break;

		int ctr = 0;
		fprintf(inpFile,"\n%lu",elemidx); ctr++;
		for(unsigned short i = 0; i < (configHd->nElemNodes)-1; i++){
			fprintf(inpFile,ctr%10 == 0 ? ",\n%lu":",%lu",elemNodes[cMap.connectivityMap[i]]);
			ctr++;
		}

		fprintf(inpFile,",%lu",elemNodes[cMap.connectivityMap[configHd->nElemNodes-1]]);
	}while(!feof(cnnFile));
	fprintf(inpFile,"\n");
	fclose(cnnFile);
	fclose(inpFile);
	printf("Volume Connectivity written\n");
}

void writeNodeSet(Config* configHd)
{
	FILE *inpFile = fopen(configHd->inpFileName,"a");
	
	FILE *nodeSetFile;
	char nodeFileName[100];
	unsigned long nodeidx;


	for(unsigned long i = 0; i < configHd->nNodesets; i++){
		fprintf(inpFile,"*NSET, NSET=%s",configHd->nodeFileList[i]);

		//Open nbc file
		sprintf(nodeFileName,"%s.%s.nbc",configHd->problemName,configHd->nodeFileList[i]);
		nodeSetFile = fopen(nodeFileName,"r");

		int ctr = 0;
		while(!feof(nodeSetFile))
		{
			fscanf(nodeSetFile,"%lu",&nodeidx);

			if(feof(nodeSetFile))
				break;

			fprintf(inpFile,ctr%10==0? "\n%u":",%u",nodeidx);
			ctr++;
		}
		fprintf(inpFile,"\n");
		fclose(nodeSetFile);
		printf("Nodeset %s written.\n",configHd->nodeFileList[i]);
	}
	fclose(inpFile);
}


int main()
{
	Config *configHd = new Config();

	//read the Configuration file
	readConfig(configHd);

	//write Problem Name
	writeHeading(configHd);

	// //Write Co-ordinates
	writeCrd(configHd);

	// //Write Connectivity
	writeCnn(configHd);

	// //Write Nodeset
	writeNodeSet(configHd);

	// //Write ElemSet
	// writeElemSet();
}
