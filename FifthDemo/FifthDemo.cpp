// FifthDemo.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include<iomanip>
#include<math.h>
using namespace std;
#include "./gdal/gdal_priv.h"

#pragma comment(lib, "gdal_i.lib")

#define TRANS_REC_NUM 3
#define BAND_I 0

void Trans(float trans[][TRANS_REC_NUM], float** rec, int n);
void debug(float** rec);

int main()
{
	char* mul_path = (char*)"America_mul.png";
	char* pan_path = (char*)"America_pan.png";
	char* fus_path = (char*)"America_fus.tif";
	float tran1[3][TRANS_REC_NUM] = { 
	{ 1.0 / 3   , 1.0/3     , 1.0 / 3 },
	{ -sqrt(2)/6, -sqrt(2)/6, sqrt(2)/3 },
	{ 1/sqrt(2) , -1/sqrt(2), 0       } };
	float tran2[3][TRANS_REC_NUM] = { 
	{ 1.0, -1 / sqrt(2), 1.0 / sqrt(2) },
	{ 1.0, -1 / sqrt(2), -1.0 / sqrt(2) },
	{ 1.0, sqrt(2)     , 0 } };
	GDALAllRegister();

	GDALDataset* poMul = (GDALDataset*)GDALOpenShared(mul_path, GA_ReadOnly);
	GDALDataset* poPan = (GDALDataset*)GDALOpenShared(pan_path, GA_ReadOnly);
	if (poMul == NULL || poPan == NULL) {
		cout << "open image file failed!" << endl;
		return 0;
	}

	int Xlen = poMul->GetRasterXSize();
	int Ylen = poMul->GetRasterYSize();
	int bandNum = poMul->GetRasterCount();

	GDALDataset* poDst = GetGDALDriverManager()->GetDriverByName("GTIFF")->Create(
		fus_path, Xlen, Ylen, bandNum, GDT_Byte, NULL);
	if (poDst == NULL) {
		cout << "create image file failed!" << endl;
		return 0;
	}

	float** imgBuf = new float*[bandNum];
	float* panBuf = (float*)CPLMalloc(Xlen * Ylen * sizeof(float));

	poPan->GetRasterBand(1)->RasterIO(GF_Read,
		0, 0, Xlen, Ylen, panBuf, Xlen, Ylen, GDT_Float32, 0, 0);

	for (int i = 0; i < bandNum; i++) {
		imgBuf[i] = (float*)CPLMalloc(Xlen * Ylen * sizeof(float));
		poMul->GetRasterBand(i + 1)->RasterIO(GF_Read,
			0, 0, Xlen, Ylen, imgBuf[i], Xlen, Ylen, GDT_Float32, 0, 0);
	}

	//RGB => IHS
	Trans(tran1, imgBuf, Xlen * Ylen);

	//replace I with PAN
	for (int i = 0; i < Xlen * Ylen; i++) {
		imgBuf[BAND_I][i] = panBuf[i];
	}

	//IHS => RGB
	Trans(tran2, imgBuf, Xlen * Ylen);

	for (int i = 0; i < bandNum; i++) {
		poDst->GetRasterBand(i + 1)->RasterIO(GF_Write, 0, 0, Xlen, Ylen, imgBuf[i], Xlen, Ylen, GDT_Float32, 0, 0);
		CPLFree(imgBuf[i]);
	}
	
	CPLFree(panBuf);
	delete[] imgBuf;

	GDALClose(poMul);
	GDALClose(poPan);
	GDALClose(poDst);

    return 0;
}

void Trans(float trans[][TRANS_REC_NUM], float** rec, int n){
	//alloc a new two-dimen array to store temp element value
	float** t = new float*[TRANS_REC_NUM];

	for (int i = 0; i < TRANS_REC_NUM; i++) {
		t[i] = new float[n];
		for (int j = 0; j < n; j++) {
			int v = 0;
			for (int temp = 0; temp < TRANS_REC_NUM; temp++) {
				v += trans[i][temp] * rec[temp][j];
			}
			t[i][j] = v;
		}
	}
	//copy value of array and then release it
	for (int i = 0; i < TRANS_REC_NUM; i++) {
		for (int j = 0; j < n; j++) {
			rec[i][j] = t[i][j];
		}
		delete[] t[i];
	}
	delete[] t;
}
void debug(float** rec) {
	cout << endl;
	for (int i = 0; i < TRANS_REC_NUM; i++) {
		for (int j = 0; j < 15; j++) {
			cout <<setw(4) << rec[i][j] << ' ';
		}
		cout << endl;
	}
	cout << endl;
}