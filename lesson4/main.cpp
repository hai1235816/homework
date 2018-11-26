#include<iostream>
using namespace std;
#include"./gdal/gdal_priv.h"
#pragma comment(lib, "gdal_i.lib")


#define BOXFILTER 1
#define MOTIONFILTER 2
#define EDGEFILTER 3
#define SHARPENFILTER 4
#define EMBOSSFILTER 5
#define GAUSSFITER 6

int imFilter(char* pathIn, char* pathOut, int flag);
//The first conv filter
int boxFilter(float* imgIn, float* imgOut, int imgXlen, int imgYlen);
//The second conv filter
int motionFilter(float* imgIn, float* imgOut, int imgXlen, int imgYlen);
//The third conv filter
int edgeFilter(float* imgIn, float* imgOut, int imgXlen, int imgYlen);
//The forth conv filter
int sharpFilter(float* imgIn, float* imgOut, int imgXlen, int imgYlen);
//The fifth conv filter
int embossFilter(float* imgIn, float* imgOut, int imgXlen, int imgYlen);
//The sixth conv filter
int gaussFilter(float* imgIn, float* imgOut, int imgXlen, int imgYlen);

int main()
{
	char* srcPath = "lena.jpg";

	GDALAllRegister();

	//The first conv filter
	imFilter(srcPath, "01_boxfilter.tif", BOXFILTER);
	//The second conv filter
	imFilter(srcPath, "02_motionfilter.tif", MOTIONFILTER);
	//The third conv filter
	imFilter(srcPath, "03_edgefilter.tif", EDGEFILTER);
	//The forth conv filter
	imFilter(srcPath, "04_sharpenfilter.tif", SHARPENFILTER);
	//The fifth conv filter
	imFilter(srcPath, "05_embossfilter.tif", EMBOSSFILTER);
	//The sixth conv filter
	imFilter(srcPath, "06_gaussfilter.tif", GAUSSFITER);
	return 0;
}

int imFilter(char* srcPath, char* dstPath, int flag)
{
	//the input dataset
	GDALDataset* poSrcDS;
	//the output dataset
	GDALDataset* poDstDS;
	int imgXlen, imgYlen, bandNum;
	int i;
	float* imgIn;
	float* imgOut;
	poSrcDS = (GDALDataset*)GDALOpenShared(srcPath, GA_ReadOnly);
	imgXlen = poSrcDS->GetRasterXSize();
	imgYlen = poSrcDS->GetRasterYSize();
	bandNum = poSrcDS->GetRasterCount();

	poDstDS = GetGDALDriverManager()->GetDriverByName("GTiff")->Create(dstPath, imgXlen, imgYlen, bandNum, GDT_Byte, NULL);
	imgIn = (float*)CPLMalloc(imgXlen * imgYlen * sizeof(float));
	imgOut = (float*)CPLMalloc(imgXlen * imgYlen * sizeof(float));

	for(i=0;i<bandNum;i++)
	{
		poSrcDS->GetRasterBand(i+1)->RasterIO(GF_Read,0, 0, imgXlen, imgYlen, imgIn, imgXlen, imgYlen, GDT_Float32, 0, 0);
		if(flag == BOXFILTER)
		{
			boxFilter(imgIn, imgOut,imgXlen, imgYlen);
		}
		else if(flag == MOTIONFILTER)
		{
			motionFilter(imgIn, imgOut,imgXlen, imgYlen);
		}
		else if(flag == EDGEFILTER)
		{
			edgeFilter(imgIn, imgOut,imgXlen, imgYlen);
		}
		else if(flag == SHARPENFILTER)
		{
			sharpFilter(imgIn, imgOut,imgXlen, imgYlen);
		}
		else if(flag == EMBOSSFILTER)
		{
			embossFilter(imgIn, imgOut,imgXlen, imgYlen);
		}
		else if(flag == GAUSSFITER)
		{
			gaussFilter(imgIn, imgOut,imgXlen, imgYlen);
		}
		poDstDS->GetRasterBand(i+1)->RasterIO(GF_Write,0, 0, imgXlen, imgYlen, imgOut, imgXlen, imgYlen, GDT_Float32, 0, 0);
	}
	CPLFree(imgIn);
	CPLFree(imgOut);
	GDALClose(poSrcDS);
	GDALClose(poDstDS);

	return 0;
}


//The first conv filter
int boxFilter(float* imgIn, float* imgOut, int imgXlen, int imgYlen)
{
	// | 0  1  0 |
    // | 1  1  1 | * 0.2
    // | 0  1  0 |
	int i, j;
	for(j=1;j<imgYlen-1;j++)
		for(i=1;i<imgXlen-1;i++)
		{
			imgOut[j*imgXlen+i]=(imgIn[(j-1)*imgXlen+i]
								+imgIn[j*imgXlen+i-1]
								+imgIn[j*imgXlen+i]
								+imgIn[j*imgXlen+i+1]
								+imgIn[(j+1)*imgXlen+i])/5.0f;
		}
	return 0;
}
//The second conv filter
int motionFilter(float* imgIn, float* imgOut, int imgXlen, int imgYlen)
{
	// | 1  0  0  0  0 |
    // | 0  1  0  0  0 |
    // | 0  0  1  0  0 | * 0.2
    // | 0  0  0  1  0 |
    // | 0  0  0  0  1 |
	int i, j;
	for(j=2;j<imgYlen-2;j++)
		for(i=2;i<imgXlen-2;i++)
		{
			imgOut[j*imgXlen+i]=(imgIn[(j-2)*imgXlen+i-2]
								+imgIn[(j-1)*imgXlen+i-1]
								+imgIn[j*imgXlen+i]
								+imgIn[(j+1)*imgXlen+i+1]
								+imgIn[(j+2)*imgXlen+i+2])/5.0f;
		}
	return 0;
}
//The third conv filter
int edgeFilter(float* imgIn, float* imgOut, int imgXlen, int imgYlen)
{
	// | -1 -1 -1 |
    // | -1  8 -1 |
    // | -1 -1 -1 |
	int i, j;
	for(j=1;j<imgYlen-1;j++)
		for(i=1;i<imgXlen-1;i++)
		{
			imgOut[j*imgXlen+i]=-imgIn[(j-1)*imgXlen+i-1]
								-imgIn[(j-1)*imgXlen+i]
								-imgIn[(j-1)*imgXlen+i+1]
								-imgIn[j*imgXlen+i-1]
								+imgIn[j*imgXlen+i]*8
								-imgIn[j*imgXlen+i+1]
								-imgIn[(j+1)*imgXlen+i-1]
								-imgIn[(j+1)*imgXlen+i]
								-imgIn[(j+1)*imgXlen+i+1];
		}
	return 0;
}
//The forth conv filter
int sharpFilter(float* imgIn, float* imgOut, int imgXlen, int imgYlen)
{
	// | -1 -1 -1 |
    // | -1  9 -1 |
    // | -1 -1 -1 |
	int i, j;
	for(j=1;j<imgYlen-1;j++)
		for(i=1;i<imgXlen-1;i++)
		{
			imgOut[j*imgXlen+i]=-imgIn[(j-1)*imgXlen+i-1]
								-imgIn[(j-1)*imgXlen+i]
								-imgIn[(j-1)*imgXlen+i+1]
								-imgIn[j*imgXlen+i-1]
								+imgIn[j*imgXlen+i]*9
								-imgIn[j*imgXlen+i+1]
								-imgIn[(j+1)*imgXlen+i-1]
								-imgIn[(j+1)*imgXlen+i]
								-imgIn[(j+1)*imgXlen+i+1];
		}
	return 0;

}
//The fifth conv filter
int embossFilter(float* imgIn, float* imgOut, int imgXlen, int imgYlen)
{
    // | -1 -1  0 |
    // | -1  0  1 |
    // |  0  1  1 |
	int i, j;
	for(j=1;j<imgYlen-1;j++)
		for(i=1;i<imgXlen-1;i++)
		{
			imgOut[j*imgXlen+i]=-imgIn[(j-1)*imgXlen+i-1]
								-imgIn[(j-1)*imgXlen+i]
								-imgIn[j*imgXlen+i-1]
								+imgIn[j*imgXlen+i+1]
								+imgIn[(j+1)*imgXlen+i]
								+imgIn[(j+1)*imgXlen+i+1];
		}
	return 0;

}
//The sixth conv filter
int gaussFilter(float* imgIn, float* imgOut, int imgXlen, int imgYlen)
{
	// | 0.0120  0.1253  0.2736  0.1253  0.0120 |
    // | 0.1253  1.3054  2.8514  1.3054  0.1253 |
    // | 0.2736  2.8514  6.2279  2.8514  0.2736 |
    // | 0.1253  1.3054  2.8514  1.3054  0.1253 |
    // | 0.0120  0.1253  0.2736  0.1253  0.0120 |
	int i, j;
	for(j=2;j<imgYlen-2;j++)
		for(i=2;i<imgXlen-2;i++)
		{
			imgOut[j*imgXlen+i]=(imgIn[(j-2)*imgXlen+i-2]*0.0120
								+imgIn[(j-2)*imgXlen+i-1]*0.1253
								+imgIn[(j-2)*imgXlen+i]  *0.2736
								+imgIn[(j-2)*imgXlen+i+1]*0.1253
								+imgIn[(j-2)*imgXlen+i+2]*0.0120

								+imgIn[(j-1)*imgXlen+i-2]*0.1253
								+imgIn[(j-1)*imgXlen+i-1]*1.3054
								+imgIn[(j-1)*imgXlen+i]  *2.8514
								+imgIn[(j-1)*imgXlen+i+1]*1.3054
								+imgIn[(j-1)*imgXlen+i+2]*0.1253

								+imgIn[j*imgXlen+i-2]*0.2736
								+imgIn[j*imgXlen+i-1]*2.8514
								+imgIn[j*imgXlen+i]  *6.2279
								+imgIn[j*imgXlen+i+1]*2.8514
								+imgIn[j*imgXlen+i+2]*0.2736

								+imgIn[(j+1)*imgXlen+i-2]*0.1253
								+imgIn[(j+1)*imgXlen+i-1]*1.3054
								+imgIn[(j+1)*imgXlen+i]  *2.8514
								+imgIn[(j+1)*imgXlen+i+1]*1.3054
								+imgIn[(j+1)*imgXlen+i+2]*0.1253

								+imgIn[(j+2)*imgXlen+i-2]*0.0120
								+imgIn[(j+2)*imgXlen+i-1]*0.1253
								+imgIn[(j+2)*imgXlen+i]  *0.2736
								+imgIn[(j+2)*imgXlen+i+1]*0.1253
								+imgIn[(j+2)*imgXlen+i+2]*0.0120)/25.0f;
		}
	return 0;
}