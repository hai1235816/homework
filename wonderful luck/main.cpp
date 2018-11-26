#include <iostream>
using namespace std;
#include "./gdal/gdal_priv.h"
#pragma comment(lib, "gdal_i.lib")

int main()
{

	//输入图像
	GDALDataset* poSrcDS1;
	GDALDataset* poSrcDS2;
	//输出图像
	GDALDataset* poDstDS;
	//输入图像路径
	char* srcPath1 = "superman.jpg";
	char* srcPath2 = "space.jpg";
	//输出图像路径
	char* dstPath = "res.tif";
	//图像内存存储
	GByte* buffTmp1R;
	GByte* buffTmp1G;
	GByte* buffTmp1B;
	GByte* buffTmp2R;
	GByte* buffTmp2G;
	GByte* buffTmp2B;
	//图像波段数
	int i,j, bandNum;
	//注册驱动
	GDALAllRegister();
	//打开图像
	poSrcDS1 = (GDALDataset*)GDALOpenShared(srcPath1,GA_ReadOnly);
	poSrcDS2 = (GDALDataset*)GDALOpenShared(srcPath2,GA_ReadOnly);
	//获取图像1波段数量
	bandNum = poSrcDS1->GetRasterCount();
	//输出获取的结果
	cout << "Image X Length: " << "640" << endl;
	cout << "Image Y Length: " << "480" << endl;
	cout << "Band number: " << bandNum << endl;
	//根据图像的宽度和高度分配内存
	buffTmp1R = (GByte*)CPLMalloc(640*480*sizeof(GByte));
	buffTmp1G = (GByte*)CPLMalloc(640*480*sizeof(GByte));
	buffTmp1B = (GByte*)CPLMalloc(640*480*sizeof(GByte));
	buffTmp2R = (GByte*)CPLMalloc(640*480*sizeof(GByte));
	buffTmp2G = (GByte*)CPLMalloc(640*480*sizeof(GByte));
	buffTmp2B = (GByte*)CPLMalloc(640*480*sizeof(GByte));
	//创键输出图像
	poDstDS = (GetGDALDriverManager()->GetDriverByName("GTiff")->Create(dstPath, 640, 480, bandNum, GDT_Byte, NULL));
	
	//buffTmp1存储的是superman.jpg
	//buffTmp2存储的是space.jpg
	poSrcDS1->GetRasterBand(1)->RasterIO(GF_Read,0, 0, 640,480, buffTmp1R, 640,480, GDT_Byte, 0, 0);
	poSrcDS1->GetRasterBand(2)->RasterIO(GF_Read,0, 0, 640,480, buffTmp1G, 640,480, GDT_Byte, 0, 0);
	poSrcDS1->GetRasterBand(3)->RasterIO(GF_Read,0, 0, 640,480, buffTmp1B, 640,480, GDT_Byte, 0, 0);
	poSrcDS2->GetRasterBand(1)->RasterIO(GF_Read,0, 0, 640,480, buffTmp2R, 640,480, GDT_Byte, 0, 0);
	poSrcDS2->GetRasterBand(2)->RasterIO(GF_Read,0, 0, 640,480, buffTmp2G, 640,480, GDT_Byte, 0, 0);
	poSrcDS2->GetRasterBand(3)->RasterIO(GF_Read,0, 0, 640,480, buffTmp2B, 640,480, GDT_Byte, 0, 0);
	for(j=0;j<480;j++)
	{
		for(i=0;i<640;i++)
		{
			int k=j*640+i;
			if((buffTmp1R[k]>10)&&(buffTmp1R[k]<160)&&
				(buffTmp1G[k]>100)&&(buffTmp1G[k]<220)&&
				(buffTmp1B[k]>10)&&(buffTmp1B[k]<110)){
					buffTmp1R[k]=buffTmp2R[k];
					buffTmp1G[k]=buffTmp2G[k];
					buffTmp1B[k]=buffTmp2B[k];
			}
		}
	}
	
	poDstDS->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, 640, 480, buffTmp1R, 640, 480, GDT_Byte, 0,0);
	poDstDS->GetRasterBand(2)->RasterIO(GF_Write, 0, 0, 640, 480, buffTmp1G, 640, 480, GDT_Byte, 0,0);
	poDstDS->GetRasterBand(3)->RasterIO(GF_Write, 0, 0, 640, 480, buffTmp1B, 640, 480, GDT_Byte, 0,0);
	printf("... ... band %d processing ... ...\n", i);
	//清除内存
	CPLFree(buffTmp1R);
	CPLFree(buffTmp1G);
	CPLFree(buffTmp1B);
	CPLFree(buffTmp2R);
	CPLFree(buffTmp2G);
	CPLFree(buffTmp2B);
	//关闭dataset
	GDALClose(poSrcDS1);
	GDALClose(poSrcDS2);
	GDALClose(poDstDS);
	system("PAUSE");
	return 0;
	}
	
	
	



	
	//思路：superman.jpg图像中的像素值用（r,g,b）表示，条件1：10<r<160; 条件2：100<g<220; 条件3：10<b<110；同时满足这三个条件的像素为背景像素。把非背景像素，填到 space.jpg 中即可。