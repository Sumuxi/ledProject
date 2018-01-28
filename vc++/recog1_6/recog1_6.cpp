#include<iostream>  
#include<opencv2/opencv.hpp>  

using namespace cv;
using namespace std;

//�ַ�����
string label[11] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "." };
//�ַ��Ƿ�ֱ
bool isVertical = true;

/*��תͼ�����ݲ��䣬�ߴ���Ӧ���
input: һ�ŷ���ֵ����ͼƬ����ʱ����ת�ĽǶ�ֵ
output����ת���ͼƬ
*/
Mat rotateImage(Mat src, int degree) {
	IplImage img = src;
	double angle = degree  * CV_PI / 180; // ����    
	double a = sin(angle), b = cos(angle);
	int width = img.width;
	int height = img.height;
	int width_rotate = int(height * fabs(a) + width * fabs(b));
	int height_rotate = int(width * fabs(a) + height * fabs(b));
	//��ת����map  
	// [ m0  m1  m2 ] ===>  [ A11  A12   b1 ]  
	// [ m3  m4  m5 ] ===>  [ A21  A22   b2 ]  
	float map[6];
	CvMat map_matrix = cvMat(2, 3, CV_32F, map);
	// ��ת����  
	CvPoint2D32f center = cvPoint2D32f(width / 2, height / 2);
	cv2DRotationMatrix(center, degree, 1.0, &map_matrix);
	map[2] += (width_rotate - width) / 2;
	map[5] += (height_rotate - height) / 2;
	IplImage* img_rotate = cvCreateImage(cvSize(width_rotate, height_rotate), 8, 1);
	//��ͼ��������任  
	//CV_WARP_FILL_OUTLIERS - ����������ͼ������ء�  
	//�������������������ͼ��ı߽��⣬��ô���ǵ�ֵ�趨Ϊ fillval.  
	//CV_WARP_INVERSE_MAP - ָ�� map_matrix �����ͼ������ͼ��ķ��任��  
	cvWarpAffine(&img, img_rotate, &map_matrix, CV_INTER_LINEAR | CV_WARP_FILL_OUTLIERS, cvScalarAll(0));
	return img_rotate;
}

/*�����ַ���б�ĽǶ�ֵ

*/
int computeAngle(Mat img) {
	IplImage src = img;
	IplImage* dst;
	IplImage* color_dst;
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* lines = 0;
	int i;
	dst = cvCreateImage(cvGetSize(&src), 8, 1);
	color_dst = cvCreateImage(cvGetSize(&src), 8, 3);
	cvCanny(&src, dst, 50, 200, 3);
	cvCvtColor(dst, color_dst, CV_GRAY2BGR);

	//����hough�任
	lines = cvHoughLines2(dst, storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI / 180, 20, 20, 10);
	double sumAngle = 0;//�Ƕ��ܺ�
	int sum = 0;//����������ֱ��������
	for (i = 0; i < lines->total; i++)
	{
		CvPoint* line = (CvPoint*)cvGetSeqElem(lines, i);
		double x = std::abs(line[0].x - line[1].x);
		double y = std::abs(line[0].y - line[1].y);
		double theta = atan2(y, x)*(180 / CV_PI);//����ֱ�����
		if (theta > 45) {
			sumAngle += theta;
			sum++;
		}
		cvLine(color_dst, line[0], line[1], CV_RGB(255, 0, 0), 3, CV_AA, 0);//��ֱ��
	}

	double avgAngle = sumAngle / sum;

	return (int)(90 - avgAngle);//תΪ�����Ƕ�
}

//���������������ַ��ָ�
vector<Mat> divideChar(Mat roi)
{
	//roi�����Ƿ���ֵ��ͼƬ
	Mat roiClone = roi.clone();//����ԴͼƬ

	//���ʹ���
	Mat dilateImg;
	Mat element = getStructuringElement(MORPH_RECT, Size(1, 20));
	dilate(roiClone, dilateImg, element);

	//�ҳ�������
	vector<vector<Point>> contours;//�ҳ��������������洢�ڴ�
	findContours(dilateImg, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	vector<Rect> rects;//�洢�ҵ������о��Σ����������ʹ�ַ�˳���Ǵ������ҵ�
	Rect rect;
	//�������ROI��һ�����ַ�
	for (uint32_t i = 0; i < contours.size(); i++)
	{
		rect = boundingRect((Mat)contours[i]);
		uint32_t j = 0;
		while (j < rects.size() && rect.x >= rects[j].x)
		{
			j++;
		}
		rects.insert(rects.begin() + j, rect);
	}

	vector<Mat> charList;//���ڴ���ָ������ÿ���ַ�
	for (uint32_t i = 0; i < rects.size(); i++)
	{
		Mat single;
		roi(rects[i]).copyTo(single);//�ü�
		charList.push_back(single);
	}
	return charList;
}

//�����������ַ��ָ�����ַ�ģ��
vector<Mat> createTemplate(Mat roi)
{
	//roi�����Ƿ���ֵ��ͼƬ
	Mat roiClone = roi.clone();//����ԴͼƬ
							   //���ʹ���
	Mat dilateImg;
	Mat element = getStructuringElement(MORPH_RECT, Size(1, 20));
	dilate(roiClone, dilateImg, element);
	//imshow("�и��ַ�ʱdilate", dilateImg);

	//�ҳ�������
	vector<vector<Point>> contours;//�ҳ��������������洢�ڴ�
	findContours(dilateImg, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	vector<Rect> rects;//�洢�ҵ������о��Σ����������ʹ�ַ�˳���Ǵ������ҵ�
	Rect rect;
	//�������ROI��һ�����ַ�
	for (uint32_t i = 0; i < contours.size(); i++)
	{
		rect = boundingRect((Mat)contours[i]);
		uint32_t j = 0;
		while (j < rects.size() && rect.x >= rects[j].x)
		{
			j++;
		}
		rects.insert(rects.begin() + j, rect);
	}

	vector<Mat> charList;//���ڴ���ָ������ÿ���ַ�
	for (uint32_t i = 0; i < rects.size(); i++)
	{
		Mat single;
		roi(rects[i]).copyTo(single);//�ü�
		charList.push_back(single);
		rectangle(roi, rects[i], Scalar(255), 2);//������

		//���ڱ����и���ַ�
		stringstream ss;
		ss << i;
		imshow(ss.str(), single);//��ʾ�ָ�����ַ�
		string picname;
		if (isVertical) {
			picname = ".\\template\\v_template\\" + ss.str() + ".jpg";//vertical templage
		}
		else
		{
			picname = ".\\template\\nv_template\\" + ss.str() + ".jpg";//non-vertical templage
		}
		imwrite(picname, single);//����ָ�����ַ�
	}
	imshow("�и��ַ�ʱ��������ַ�", roi);
	return charList;
}

/*ģ��ƥ�䷨
*/

//��ȡģ��
vector<Mat> getMatList()
{
	string path;
	if (isVertical) {
		path = ".\\template\\v_template\\";//vertical templage
	}
	else
	{
		path = ".\\template\\nv_template\\";//non-vertical templage
	}
	Mat img;
	vector<Mat> matList;
	for (int i = 0; i <= 10; i++)
	{
		stringstream ss;
		ss << path << i << ".jpg";
		img = imread(ss.str(), 0);
		matList.push_back(img);
	}
	return matList;
}

//�������ƶ�
double getSimilar2(Mat img1, Mat img2)		//img1��ģ��
{

	threshold(img1, img1, 150, 255, THRESH_BINARY);
	threshold(img2, img2, 150, 255, THRESH_BINARY);
	//Mat cutImg = getCutMat(img2);
	int height = img1.rows;
	int width = img1.cols;
	Mat normImg = Mat::zeros(width, height, CV_8UC1);
	resize(img2, normImg, Size(width, height), CV_INTER_LINEAR);//��һ��
	threshold(normImg, normImg, 150, 255, THRESH_BINARY);
	double similar;
	int count = 0;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (abs(img1.at<uchar>(i, j) - normImg.at<uchar>(i, j)) == 0)
			{
				count++;
			}
		}
	}
	similar = count*1.0 / (height*width);
	return similar;
}

/*�ַ�ƥ�䣬ѡ���ƶ�����
�������ƶ�����ģ�������
���������ƶ�С��80%������-1
*/
int matchCharactor2(Mat img, vector<Mat> matList)
{
	double max = 0.0;//������ƶ�
	double similar;

	int index = 0;
	for (uint32_t i = 0; i < matList.size(); i++)
	{
		similar = getSimilar2(matList[i], img);
		cout << "�͵�" << i << "���ƶ�Ϊ��" << similar << endl;
		if (similar > max)
		{
			max = similar;
			index = i;
		}
	}

	if (max <= 0.8) {//���ƶ�С��80%
		return -1;
	}

	return index;
}

/*�ж�ͼƬ�ǲ���black_n_whiteͼƬ���ڱ�����ǰ����
*/
bool is_black_n_white(Mat src)
{
	IplImage *input = &(IplImage)src; /*MatתIplImage,ֻ����ͼ��ͷ������������*/

	int width = input->width;
	int height = input->height;

	int num_w = 0, num_b = 0;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++)
		{
			CvScalar pixel = cvGet2D(input, i, j);//��ȡ���ص�Ϊ��j, i�����HSV��ֵ  

			if (pixel.val[0] == 255)//white
			{
				num_w++;
			}
			if (pixel.val[0] == 0)//black
			{
				num_b++;
			}
		}
	}

	if (num_b > num_w) {
		return true;
	}
	else {
		return false;
	}
}

/*����DLL�пɱ�C#���õĺ���
	�õ�ʶ����
*/
extern "C" __declspec(dllexport) 
char * recognize(char * filename) {
	Mat src = imread(filename, IMREAD_COLOR);

	//�ҶȻ�
	Mat grayImg;
	cvtColor(src, grayImg, CV_BGR2GRAY);

	//��Ե���
	Mat edges;
	Canny(grayImg, edges, 50, 200, 3);

	//��Ե�����ʹ���
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));//���ʹ����kernel,���������������ʾĬ��Ϊ�˵�����
	Mat dilateImg;//�Ƿ���ֵ��ͼƬ
	dilate(edges, dilateImg, element);

	//ȥ��С���ص�
	vector<vector<Point>> contours;//����
	Mat removeDot = dilateImg.clone();
	findContours(dilateImg, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	for (uint32_t i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect((Mat)contours[i]);
		if (rect.height <= 15 && rect.width <= 15) {
			drawContours(removeDot, contours, i, Scalar(0), -1);//С���ص�ͿΪ��ɫ
		}
	}

	//��ֱ�������ʹ���
	Mat element1 = getStructuringElement(MORPH_RECT, Size(1, 20));//���ʹ����kernel,���������������ʾĬ��Ϊ�˵�����
	Mat dilateImg1;//Ҳ�Ƿ���ֵ��ͼƬ
	dilate(removeDot, dilateImg1, element1);

	//�ҳ����ʹ�����ÿ���ַ�
	Mat verticalImg = Mat::zeros(edges.rows, edges.cols, CV_8U);
	contours.clear();
	findContours(dilateImg1, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);//CV_CHAIN_APPROX_SIMPLE
	double ratio;
	for (uint32_t i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect((Mat)contours[i]);
		ratio = (double)rect.height / (double)rect.width;
		if (ratio>1 && (rect.width>15 || rect.height>15))//�߿�ȴ���1�ҸߺͿ���ͬʱС��15����С��15����������
			rectangle(verticalImg, rect, Scalar(255), -1);
	}

	//ˮƽ�������ʹ������Ϳ����΢��һЩӰ��Ҳ����
	Mat element2 = getStructuringElement(MORPH_RECT, Size(60, 1));//���ʹ����kernel,���������������ʾĬ��Ϊ�˵�����
	Mat dilateImg2;
	dilate(verticalImg, dilateImg2, element2);//ʹ�������element��binary_inv�����ʹ���(���Ͱ�ɫ����)

	contours.clear();
	findContours(dilateImg2, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);//�ҳ�ͼƬ���ڱ�����ǰ�������ͺ������

	Rect maxRect(0, 0, 0, 0);//���Ϊ0�ľ���
	for (uint32_t i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect((Mat)contours[i]);
		if (rect.area() >= maxRect.area()) {
			maxRect = rect;
		}
	}
	if (maxRect.height <= 50 && maxRect.width <= 60)//�ҵ�ROI
		return NULL;//no roi

	//�õ�����ֵ����ROIͼ
	Mat roi_src;
	src(maxRect).copyTo(roi_src);
	Mat roi_gray;
	cvtColor(roi_src, roi_gray, CV_BGR2GRAY);
	Mat roi;
	threshold(roi_gray, roi, 0, 255, CV_THRESH_OTSU);
	//�ж�roi�ǲ��Ǻڰ�ͼ
	if (!is_black_n_white(roi)) {
		threshold(roi, roi, 100, 255, CV_THRESH_BINARY_INV);//����ֵ��
	}

	//��бУ�����ж��ַ��ǲ��Ǵ�ֱ��
	int angle = computeAngle(roi.clone());
	if (std::abs(angle) > 4)//��Ǵ���4�ȣ�����ת���ַ���Ϊ��ֱ
	{
		isVertical = false;
		roi = rotateImage(roi, angle);
	}

	vector<Mat> matList;//���ڴ洢�ָ�������е����ַ�
	matList = divideChar(roi);

	vector<Mat> templeteMatList = getMatList();

	string result = "";
	for (uint32_t i = 0; i < matList.size(); i++)
	{
		int index = matchCharactor2(matList[i], templeteMatList);
		if (index < 0) {
			continue;
		}
		result.append(label[index]);
	}

	const size_t len = result.length();
	char * arr = new char[len + 1];
	for (size_t i = 0; i < len; i++) {
		arr[i] = result.at(i);
	}
	arr[len] = '\0';
	return arr;
}

/*�ҳ�ԭͼ��ROI������ʾ�м���̣����ڲ���
*/
void findROI(char * filename) {
	Mat src = imread(filename, IMREAD_COLOR);
	imshow("ԭͼ", src);

	//�ҶȻ�
	Mat grayImg;
	cvtColor(src, grayImg, CV_BGR2GRAY);
	imshow("�Ҷ�ͼ", grayImg);

	//��Ե���
	Mat edges;
	Canny(grayImg, edges, 50, 200, 3);
	imshow("��Ե���", edges);

	//��Ե�����ʹ���
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));//���ʹ����kernel,���������������ʾĬ��Ϊ�˵�����
	Mat dilateImg;//�Ƿ���ֵ��ͼƬ
	dilate(edges, dilateImg, element);
	imshow("��Ե���ʹ���", dilateImg);

	//ȥ��С���ص�
	vector<vector<Point>> contours;//����
	Mat removeDot = dilateImg.clone();
	findContours(dilateImg, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	for (uint32_t i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect((Mat)contours[i]);
		if (rect.height <= 15 && rect.width <= 15) {
			drawContours(removeDot, contours, i, Scalar(0), -1);//С���ص�ͿΪ��ɫ
		}
	}
	imshow("ȥ��С���ص�", removeDot);

	//��ֱ�������ʹ���
	Mat element1 = getStructuringElement(MORPH_RECT, Size(1, 20));//���ʹ����kernel,���������������ʾĬ��Ϊ�˵�����
	Mat dilateImg1;//Ҳ�Ƿ���ֵ��ͼƬ
	dilate(removeDot, dilateImg1, element1);
	imshow("��ֱ�������ʹ���", dilateImg1);

	//�ҳ���ֱ�������ʹ�����ÿ���ַ�
	Mat verticalImg = Mat::zeros(edges.rows, edges.cols, CV_8U);
	contours.clear();
	findContours(dilateImg1, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);//CV_CHAIN_APPROX_SIMPLE
	double ratio;
	for (uint32_t i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect((Mat)contours[i]);
		ratio = (double)rect.height / (double)rect.width;
		if (ratio>1 && (rect.width>15 || rect.height>15))//�߿�ȴ���1�ҸߺͿ���ͬʱС��15����С��15����������
			rectangle(verticalImg, rect, Scalar(255), -1);
	}
	imshow("���ÿ���ַ�", verticalImg);

	//ˮƽ�������ʹ������Ϳ����΢��һЩӰ��Ҳ����
	Mat element2 = getStructuringElement(MORPH_RECT, Size(60, 1));
	Mat dilateImg2;
	dilate(verticalImg, dilateImg2, element2);
	imshow("ˮƽ�������ʹ���", dilateImg2);

	//�ҳ����µ���������
	Mat horizontalImg = Mat::zeros(edges.rows, edges.cols, CV_8U);
	contours.clear();
	findContours(dilateImg2, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);//�ҳ�ͼƬ���ڱ�����ǰ�������ͺ������

	Rect maxRect(0, 0, 0, 0);//���Ϊ0�ľ���
	for (uint32_t i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect((Mat)contours[i]);
		if (rect.area() >= maxRect.area()) {
			maxRect = rect;
		}

		rectangle(horizontalImg, rect, Scalar(255), 1);
	}
	imshow("ˮƽ�������ʹ������ַ�����", horizontalImg);

	if (maxRect.height <= 50 && maxRect.width <= 60)
		return;//no roi

	//�õ�����ֵ����ROIͼ
	Mat roi_src;
	src(maxRect).copyTo(roi_src);
	imshow("��ԭͼ�ϲü���ROI", roi_src);
	Mat roi_gray;
	cvtColor(roi_src, roi_gray, CV_BGR2GRAY);
	Mat roi;
	threshold(roi_gray, roi, 0, 255, CV_THRESH_OTSU);
	imshow("roi��otsu��ֵ��", roi);
	//�ж�roi�ǲ��Ǻڰ�ͼ
	if (!is_black_n_white(roi)) {
		threshold(roi, roi, 100, 255, CV_THRESH_BINARY_INV);//����ֵ��
	}
	imshow("���յķ���ֵ��roi", roi);
	//��бУ�����ж��ַ��ǲ��Ǵ�ֱ��
	//�ж��ַ��ǲ��Ǵ�ֱ��
	int angle = computeAngle(roi.clone());
	if (std::abs(angle) <= 4) {//���С��4�ȣ�����ת���ַ���Ϊ��ֱ
		cout << "�ַ��� ��ֱ ��" << endl;
	}
	else//�ַ���Ϊ��б
	{
		isVertical = false;
		roi = rotateImage(roi, angle);
		cout << "�ַ��� ��б ��" << endl;
	}
	imshow("��бУ��", roi);

	//ȡ��ע�Ϳ��Էָ��ַ�����ģ�壬ģ��ͼ���밴˳����� 0123456789. ʮ���ַ�
	//createTemplate(roi);
}

int main() {
	findROI(".\\template\\sample8.jpg");

	cout << recognize(".\\template\\sample8.jpg") << endl;

	waitKey(0);
	return 0;
}

