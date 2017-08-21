#include"stdafx.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include<fstream>
#include<string>

using namespace std;
using namespace cv;

const int imageWidth = 2992;                             //����ͷ�ķֱ���  
const int imageHeight = 2000;
const int boardWidth = 7;                               //����Ľǵ���Ŀ  
const int boardHeight = 6;                              //����Ľǵ�����  
const int boardCorner = boardWidth * boardHeight;       //�ܵĽǵ�����  
const int frameNumber = 18;                             //����궨ʱ��Ҫ���õ�ͼ��֡��  
const int squareSize = 23;                              //�궨��ڰ׸��ӵı߳� ��λmm  
const Size boardSize = Size(boardWidth, boardHeight);   //�궨��ǵ�

char namestring[30] = "G:\\im\\IMG%d_.jpg";

Mat intrinsic(3, 3, CV_64FC1, Scalar::all(0.0));		//����ڲξ���  
Mat distortion_coeff(8, 1, CV_64FC1, Scalar::all(0.0)); //���������� 
vector<Mat> rvecs;                                      //��ת����  
vector<Mat> tvecs;                                      //ƽ������  
vector<vector<Point2f>> corners;                        //����ͼ���ҵ��Ľǵ�ļ��� ��objRealPoint һһ��Ӧ  
vector<vector<Point3f>> objRealPoint;                   //����ͼ��Ľǵ��ʵ���������꼯��  
vector<Point2f> corner;                                 //ĳһ��ͼ��Ľǵ�  

Mat rgbImage, grayImage;

//����궨����ģ���ʵ����������
void calRealPoint(vector<vector<Point3f>>& obj, int boardwidth, int boardheight, int imgNumber, int squaresize)
{
	//  Mat imgpoint(boardheight, boardwidth, CV_32FC3,Scalar(0,0,0));  
	vector<Point3f> imgpoint;
	for (int rowIndex = 0; rowIndex < boardheight; rowIndex++)
		for (int colIndex = 0; colIndex < boardwidth; colIndex++)
			imgpoint.push_back(Point3f(float(colIndex * squaresize), float(rowIndex * squaresize), 0));		//z����Ϊ0���ٶ�����zƽ���ϣ�
	obj.clear();
	obj.resize(imgNumber, imgpoint);
}

	ofstream fout("G:\\im\\camera_params.txt");		//������ļ�

void outputCameraParam(void)
{
	/*�ڲξ���
		fx 0  0
		0  fy 0
		cx cy 1
	*/
	cout << "fx :" << intrinsic.at<double>(0, 0) << endl << "fy :" << intrinsic.at<double>(1, 1) << endl;
	cout << "cx :" << intrinsic.at<double>(0, 2) << endl << "cy :" << intrinsic.at<double>(1, 2) << endl;

	cout << "k1 :" << distortion_coeff.at<double>(0, 0) << endl;
	cout << "k2 :" << distortion_coeff.at<double>(1, 0) << endl;
	cout << "p1 :" << distortion_coeff.at<double>(2, 0) << endl;
	cout << "p2 :" << distortion_coeff.at<double>(3, 0) << endl;
	cout << "k3 :" << distortion_coeff.at<double>(4, 0) << endl;

	fout << "fx :" << intrinsic.at<double>(0, 0) << endl << "fy :" << intrinsic.at<double>(1, 1) << endl;
	fout << "cx :" << intrinsic.at<double>(0, 2) << endl << "cy :" << intrinsic.at<double>(1, 2) << endl;

	fout << "k1 :" << distortion_coeff.at<double>(0, 0) << endl;
	fout << "k2 :" << distortion_coeff.at<double>(1, 0) << endl;
	fout << "p1 :" << distortion_coeff.at<double>(2, 0) << endl;
	fout << "p2 :" << distortion_coeff.at<double>(3, 0) << endl;
	fout << "k3 :" << distortion_coeff.at<double>(4, 0) << endl;
	fout << flush;
}

//�궨�������������
void CalibrationEvaluate(void)
{
	double err = 0;
	double total_err = 0;
	cout << "ÿ��ͼ��Ķ�����" << endl;
	for (int i = 0; i < corners.size(); i++)
	{
		vector<Point2f> image_points2;
		vector<Point3f> tempPointSet = objRealPoint[i];
		projectPoints(tempPointSet, rvecs[i], tvecs[i], intrinsic, distortion_coeff, image_points2);

		vector<Point2f> tempImagePoint = corners[i];
		Mat tempImagePointMat = Mat(1, tempImagePoint.size(), CV_32FC2);
		Mat image_points2Mat = Mat(1, image_points2.size(), CV_32FC2);
		for (int j = 0; j < tempImagePoint.size(); j++)
		{
			image_points2Mat.at<Vec2f>(0, j) = Vec2f(image_points2[j].x, image_points2[j].y);
			tempImagePointMat.at<Vec2f>(0, j) = Vec2f(tempImagePoint[j].x, tempImagePoint[j].y);
		}
		err = norm(image_points2Mat, tempImagePointMat, NORM_L2);
		total_err = err + total_err;
		cout << "��" << i + 1 << "��ͼ���ƽ����" << err << "����" << endl;
		fout<<endl<<endl<< "��" << i + 1 << "��ͼ���ƽ����" << err << "����" << endl;
	}
	cout << "����ƽ����" << total_err / (corners.size() + 1) << "����" << endl;
	fout<<endl<< "����ƽ����" << total_err / (corners.size() + 1) << "����" << endl;
	fout << flush;
	fout.close();
}


int main()
{
	Mat img;
	int goodFrameCount = 1;
	while (goodFrameCount <= frameNumber)
	{
		char filename[50];
		sprintf_s(filename, namestring, 9 + goodFrameCount);
		goodFrameCount++;
		rgbImage = imread(filename, 1); 
		grayImage = imread(filename, 0);
		imshow("Camera", grayImage);cvWaitKey(40);
		//Ѱ�ҽǵ�
		if (findChessboardCorners(rgbImage, boardSize, corner)== true) //���нǵ㶼���ҵ� ˵�����ͼ���ǿ��е�  
		{
			cout << "The image is good" << endl;
			cornerSubPix(grayImage, corner, Size(8, 8), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));		//�ǵ��Ż�
			drawChessboardCorners(rgbImage, boardSize, corner, 1);		//���ƽǵ�
			imshow("chessboard", rgbImage); cvWaitKey(40);
			corners.push_back(corner);
		}
		else
		{
			cout << "The image is bad please try again" << endl;
		}
	}
	/*����ʵ�ʵ�У�������ά����*/
	calRealPoint(objRealPoint, boardWidth, boardHeight, frameNumber, squareSize);
	cout << "calculate real coordinates successful" << endl;
	/*�궨����ͷ*/
	calibrateCamera(objRealPoint, corners, Size(imageWidth, imageHeight), intrinsic, distortion_coeff, rvecs, tvecs, CV_CALIB_ZERO_TANGENT_DIST);   //CV_CALIB_ZERO_TANGENT_DIST
	cout << "calibration successful" << endl;
	/*���沢�������*/  
	outputCameraParam();
	CalibrationEvaluate();
	cout << "out successful" << endl;

	/*��ʾ����У��Ч��*/
	Mat cImage;
	undistort(rgbImage, cImage, intrinsic, distortion_coeff);
	imshow("noCorrect Image", rgbImage); cvWaitKey(40);
	imshow("Correct Image", cImage); cvWaitKey(40);
	imwrite("G:\\im\\no_corrected.jpg", rgbImage);
	imwrite("G:\\im\\corrected.jpg", cImage);

	cvWaitKey(0);
	system("pause");
	return 0;
}