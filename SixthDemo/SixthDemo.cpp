// SixthDemo.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include<time.h>
using namespace std;
#include "./gdal/gdal_priv.h"

#define TRANS_REC_NUM 3
#define BLOCK_SIZE 256
#define SQUARE_DIVISION 1
#define MATCH_DIVISION 2
#define SINGLE_BAND 1
#define BAND_I 0
#define X_REMAIN 1
#define Y_REMAIN 1

#pragma comment(lib, "gdal_i.lib")

void Trans(float trans[][TRANS_REC_NUM], float** rec, int n);

int main()
{
	clock_t start;
	char* mul_path = (char*)"Mul_large.tif";
	char* pan_path = (char*)"Pan_large.tif";
	char* fus_path = (char*)"Fus_large.tif";
	float tran1[3][TRANS_REC_NUM] = {
	{ 1.0 / 3   , 1.0 / 3     , 1.0 / 3 },
	{ -sqrt(2) / 6, -sqrt(2) / 6, sqrt(2) / 3 },
	{ 1 / sqrt(2) , -1 / sqrt(2), 0 } };
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
	int XblockNum = Xlen / BLOCK_SIZE;
	int Xremain = Xlen % BLOCK_SIZE;
	int YblockNum = Ylen / BLOCK_SIZE;
	int Yremain = Ylen % BLOCK_SIZE;

	GDALDataset* poFus = GetGDALDriverManager()->GetDriverByName("GTIFF")->Create(
		fus_path, Xlen, Ylen, bandNum, GDT_Byte, NULL);
	float** imgBuf = new float*[bandNum];

	for (int flag = 1; flag < 3; flag++) {

		start = clock();
		if (flag == SQUARE_DIVISION) {
			cout << "processing by Square Division:" << endl;
			//process data block by block
			int Xsize, Ysize;
			for (int yb = 0; yb < YblockNum + Y_REMAIN; yb++) {

				Ysize = yb == YblockNum ? Yremain : BLOCK_SIZE;
				for (int xb = 0; xb < XblockNum + X_REMAIN; xb++) {

					Xsize = xb == XblockNum ? Xremain : BLOCK_SIZE;
					//process each band of image
					for (int i = 0; i < bandNum; i++) {
						imgBuf[i] = (float*)CPLMalloc(Xsize*Ysize * sizeof(float));
						poMul->GetRasterBand(i + 1)->RasterIO(GF_Read, xb*BLOCK_SIZE, yb*BLOCK_SIZE,
							Xsize, Ysize, imgBuf[i], Xsize, Ysize, GDT_Float32, 0, 0);
					}
					//alloc memory for Pan_large block image
					float* panBuf = (float*)CPLMalloc(Xsize*Ysize * sizeof(float));
					poPan->GetRasterBand(SINGLE_BAND)->RasterIO(GF_Read, xb*BLOCK_SIZE, yb*BLOCK_SIZE,
						Xsize, Ysize, panBuf, Xsize, Ysize, GDT_Float32, 0, 0);

					//RGB => IHS
					Trans(tran1, imgBuf, Xsize*Ysize);

					//replace I with PAN
					for (int i = 0; i < Xsize*Ysize; i++) {
						imgBuf[BAND_I][i] = panBuf[i];
					}

					//IHS => RGB
					Trans(tran2, imgBuf, Xsize*Ysize);

					for (int i = 0; i < bandNum; i++) {
						poFus->GetRasterBand(i + 1)->RasterIO(GF_Write, xb*BLOCK_SIZE, yb*BLOCK_SIZE, Xsize, Ysize, imgBuf[i], Xsize, Ysize, GDT_Float32, 0, 0);
						CPLFree(imgBuf[i]);
					}
					CPLFree(panBuf);
				}
			}
			cout << "Square Division costs " << double(clock() - start) / CLOCKS_PER_SEC << "(s)" << endl;
			continue;
		}

		start = clock();
		if (flag == MATCH_DIVISION) {
			cout << "processing by Match Division:" << endl;
			//process data block by block
			int Ysize;
			for (int yb = 0; yb < YblockNum + Y_REMAIN; yb++) {
				Ysize = yb == YblockNum ? Yremain : BLOCK_SIZE;
				//process each band of image 
				for (int i = 0; i < bandNum; i++) {
					imgBuf[i] = (float*)CPLMalloc(Xlen*Ysize * sizeof(float));
					poMul->GetRasterBand(i + 1)->RasterIO(GF_Read, 0, yb*BLOCK_SIZE,
						Xlen, Ysize, imgBuf[i], Xlen, Ysize, GDT_Float32, 0, 0);
				}
				//alloc memory for Pan_large block image
				float* panBuf = (float*)CPLMalloc(Xlen*Ysize * sizeof(float));
				poPan->GetRasterBand(SINGLE_BAND)->RasterIO(GF_Read, 0, yb*BLOCK_SIZE,
					Xlen, Ysize, panBuf, Xlen, Ysize, GDT_Float32, 0, 0);

				//RGB => IHS
				Trans(tran1, imgBuf, Xlen*Ysize);

				//replace I with PAN
				for (int i = 0; i < Xlen*Ysize; i++) {
					imgBuf[BAND_I][i] = panBuf[i];
				}

				//IHS => RGB
				Trans(tran2, imgBuf, Xlen*Ysize);

				for (int i = 0; i < bandNum; i++) {
					poFus->GetRasterBand(i + 1)->RasterIO(GF_Write, 0, yb*BLOCK_SIZE, Xlen, Ysize, imgBuf[i], Xlen, Ysize, GDT_Float32, 0, 0);
					CPLFree(imgBuf[i]);
				}
				CPLFree(panBuf);
			}
			cout << "Match Division costs " << double(clock() - start) / CLOCKS_PER_SEC << "(s)" << endl;
			continue;
		}
	}

	delete[] imgBuf;

	GDALClose(poMul);
	GDALClose(poPan);
	GDALClose(poFus);
    return 0;
}

void Trans(float trans[][TRANS_REC_NUM], float** rec, int n) {
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