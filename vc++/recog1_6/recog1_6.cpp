#include<iostream>  
#include<opencv2/opencv.hpp>  

using namespace cv;
using namespace std;

//字符常量
string label[11] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "." };
//字符是否垂直
bool isVertical = true;

/*旋转图像内容不变，尺寸相应变大
input: 一张反二值化的图片，逆时针旋转的角度值
output：旋转后的图片
*/
Mat rotateImage(Mat src, int degree) {
	IplImage img = src;
	double angle = degree  * CV_PI / 180; // 弧度    
	double a = sin(angle), b = cos(angle);
	int width = img.width;
	int height = img.height;
	int width_rotate = int(height * fabs(a) + width * fabs(b));
	int height_rotate = int(width * fabs(a) + height * fabs(b));
	//旋转数组map  
	// [ m0  m1  m2 ] ===>  [ A11  A12   b1 ]  
	// [ m3  m4  m5 ] ===>  [ A21  A22   b2 ]  
	float map[6];
	CvMat map_matrix = cvMat(2, 3, CV_32F, map);
	// 旋转中心  
	CvPoint2D32f center = cvPoint2D32f(width / 2, height / 2);
	cv2DRotationMatrix(center, degree, 1.0, &map_matrix);
	map[2] += (width_rotate - width) / 2;
	map[5] += (height_rotate - height) / 2;
	IplImage* img_rotate = cvCreateImage(cvSize(width_rotate, height_rotate), 8, 1);
	//对图像做仿射变换  
	//CV_WARP_FILL_OUTLIERS - 填充所有输出图像的象素。  
	//如果部分象素落在输入图像的边界外，那么它们的值设定为 fillval.  
	//CV_WARP_INVERSE_MAP - 指定 map_matrix 是输出图像到输入图像的反变换，  
	cvWarpAffine(&img, img_rotate, &map_matrix, CV_INTER_LINEAR | CV_WARP_FILL_OUTLIERS, cvScalarAll(0));
	return img_rotate;
}

/*计算字符倾斜的角度值

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

	//概率hough变换
	lines = cvHoughLines2(dst, storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI / 180, 20, 20, 10);
	double sumAngle = 0;//角度总和
	int sum = 0;//符合条件的直线总条数
	for (i = 0; i < lines->total; i++)
	{
		CvPoint* line = (CvPoint*)cvGetSeqElem(lines, i);
		double x = std::abs(line[0].x - line[1].x);
		double y = std::abs(line[0].y - line[1].y);
		double theta = atan2(y, x)*(180 / CV_PI);//计算直线倾角
		if (theta > 45) {
			sumAngle += theta;
			sum++;
		}
		cvLine(color_dst, line[0], line[1], CV_RGB(255, 0, 0), 3, CV_AA, 0);//画直线
	}

	double avgAngle = sumAngle / sum;

	return (int)(90 - avgAngle);//转为整数角度
}

//查找轮廓法进行字符分割
vector<Mat> divideChar(Mat roi)
{
	//roi必须是反二值化图片
	Mat roiClone = roi.clone();//复制源图片

	//膨胀处理
	Mat dilateImg;
	Mat element = getStructuringElement(MORPH_RECT, Size(1, 20));
	dilate(roiClone, dilateImg, element);

	//找出外轮廓
	vector<vector<Point>> contours;//找出的所有轮廓将存储于此
	findContours(dilateImg, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	vector<Rect> rects;//存储找到的所有矩形，需进行排序，使字符顺序是从左至右的
	Rect rect;
	//这里假设ROI上一定有字符
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

	vector<Mat> charList;//用于储存分割出来的每个字符
	for (uint32_t i = 0; i < rects.size(); i++)
	{
		Mat single;
		roi(rects[i]).copyTo(single);//裁剪
		charList.push_back(single);
	}
	return charList;
}

//查找轮廓法字符分割，生成字符模板
vector<Mat> createTemplate(Mat roi)
{
	//roi必须是反二值化图片
	Mat roiClone = roi.clone();//复制源图片
							   //膨胀处理
	Mat dilateImg;
	Mat element = getStructuringElement(MORPH_RECT, Size(1, 20));
	dilate(roiClone, dilateImg, element);
	//imshow("切割字符时dilate", dilateImg);

	//找出外轮廓
	vector<vector<Point>> contours;//找出的所有轮廓将存储于此
	findContours(dilateImg, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	vector<Rect> rects;//存储找到的所有矩形，需进行排序，使字符顺序是从左至右的
	Rect rect;
	//这里假设ROI上一定有字符
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

	vector<Mat> charList;//用于储存分割出来的每个字符
	for (uint32_t i = 0; i < rects.size(); i++)
	{
		Mat single;
		roi(rects[i]).copyTo(single);//裁剪
		charList.push_back(single);
		rectangle(roi, rects[i], Scalar(255), 2);//画白线

		//用于保存切割的字符
		stringstream ss;
		ss << i;
		imshow(ss.str(), single);//显示分割出的字符
		string picname;
		if (isVertical) {
			picname = ".\\template\\v_template\\" + ss.str() + ".jpg";//vertical templage
		}
		else
		{
			picname = ".\\template\\nv_template\\" + ss.str() + ".jpg";//non-vertical templage
		}
		imwrite(picname, single);//保存分割出的字符
	}
	imshow("切割字符时标记所有字符", roi);
	return charList;
}

/*模板匹配法
*/

//获取模板
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

//计算相似度
double getSimilar2(Mat img1, Mat img2)		//img1是模板
{

	threshold(img1, img1, 150, 255, THRESH_BINARY);
	threshold(img2, img2, 150, 255, THRESH_BINARY);
	//Mat cutImg = getCutMat(img2);
	int height = img1.rows;
	int width = img1.cols;
	Mat normImg = Mat::zeros(width, height, CV_8UC1);
	resize(img2, normImg, Size(width, height), CV_INTER_LINEAR);//归一化
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

/*字符匹配，选相似度最大的
返回相似度最大的模板的索引
如果最大相似度小于80%，返回-1
*/
int matchCharactor2(Mat img, vector<Mat> matList)
{
	double max = 0.0;//最大相似度
	double similar;

	int index = 0;
	for (uint32_t i = 0; i < matList.size(); i++)
	{
		similar = getSimilar2(matList[i], img);
		cout << "和第" << i << "相似度为：" << similar << endl;
		if (similar > max)
		{
			max = similar;
			index = i;
		}
	}

	if (max <= 0.8) {//相似度小于80%
		return -1;
	}

	return index;
}

/*判断图片是不是black_n_white图片（黑背景白前景）
*/
bool is_black_n_white(Mat src)
{
	IplImage *input = &(IplImage)src; /*Mat转IplImage,只创建图像头，不复制数据*/

	int width = input->width;
	int height = input->height;

	int num_w = 0, num_b = 0;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++)
		{
			CvScalar pixel = cvGet2D(input, i, j);//获取像素点为（j, i）点的HSV的值  

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

/*定义DLL中可被C#调用的函数
	得到识别结果
*/
extern "C" __declspec(dllexport) 
char * recognize(char * filename) {
	Mat src = imread(filename, IMREAD_COLOR);

	//灰度化
	Mat grayImg;
	cvtColor(src, grayImg, CV_BGR2GRAY);

	//边缘检测
	Mat edges;
	Canny(grayImg, edges, 50, 200, 3);

	//边缘做膨胀处理
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));//膨胀处理的kernel,第三个参数不填表示默认为核的中心
	Mat dilateImg;//是反二值化图片
	dilate(edges, dilateImg, element);

	//去除小像素点
	vector<vector<Point>> contours;//轮廓
	Mat removeDot = dilateImg.clone();
	findContours(dilateImg, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	for (uint32_t i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect((Mat)contours[i]);
		if (rect.height <= 15 && rect.width <= 15) {
			drawContours(removeDot, contours, i, Scalar(0), -1);//小像素点涂为黑色
		}
	}

	//垂直方向膨胀处理
	Mat element1 = getStructuringElement(MORPH_RECT, Size(1, 20));//膨胀处理的kernel,第三个参数不填表示默认为核的中心
	Mat dilateImg1;//也是反二值化图片
	dilate(removeDot, dilateImg1, element1);

	//找出膨胀处理后的每个字符
	Mat verticalImg = Mat::zeros(edges.rows, edges.cols, CV_8U);
	contours.clear();
	findContours(dilateImg1, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);//CV_CHAIN_APPROX_SIMPLE
	double ratio;
	for (uint32_t i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect((Mat)contours[i]);
		ratio = (double)rect.height / (double)rect.width;
		if (ratio>1 && (rect.width>15 || rect.height>15))//高宽比大于1且高和宽不能同时小于15（都小于15的是噪声）
			rectangle(verticalImg, rect, Scalar(255), -1);
	}

	//水平方向膨胀处理，膨胀宽度稍微大一些影响也不大
	Mat element2 = getStructuringElement(MORPH_RECT, Size(60, 1));//膨胀处理的kernel,第三个参数不填表示默认为核的中心
	Mat dilateImg2;
	dilate(verticalImg, dilateImg2, element2);//使用上面的element对binary_inv做膨胀处理(膨胀白色像素)

	contours.clear();
	findContours(dilateImg2, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);//找出图片（黑背景白前景）膨胀后的轮廓

	Rect maxRect(0, 0, 0, 0);//面积为0的矩形
	for (uint32_t i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect((Mat)contours[i]);
		if (rect.area() >= maxRect.area()) {
			maxRect = rect;
		}
	}
	if (maxRect.height <= 50 && maxRect.width <= 60)//找到ROI
		return NULL;//no roi

	//得到反二值化的ROI图
	Mat roi_src;
	src(maxRect).copyTo(roi_src);
	Mat roi_gray;
	cvtColor(roi_src, roi_gray, CV_BGR2GRAY);
	Mat roi;
	threshold(roi_gray, roi, 0, 255, CV_THRESH_OTSU);
	//判断roi是不是黑白图
	if (!is_black_n_white(roi)) {
		threshold(roi, roi, 100, 255, CV_THRESH_BINARY_INV);//反二值化
	}

	//倾斜校正，判断字符是不是垂直的
	int angle = computeAngle(roi.clone());
	if (std::abs(angle) > 4)//倾角大于4度，不旋转，字符视为垂直
	{
		isVertical = false;
		roi = rotateImage(roi, angle);
	}

	vector<Mat> matList;//用于存储分割出的所有单个字符
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

/*找出原图的ROI区域，显示中间过程，用于测试
*/
void findROI(char * filename) {
	Mat src = imread(filename, IMREAD_COLOR);
	imshow("原图", src);

	//灰度化
	Mat grayImg;
	cvtColor(src, grayImg, CV_BGR2GRAY);
	imshow("灰度图", grayImg);

	//边缘检测
	Mat edges;
	Canny(grayImg, edges, 50, 200, 3);
	imshow("边缘检测", edges);

	//边缘做膨胀处理
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));//膨胀处理的kernel,第三个参数不填表示默认为核的中心
	Mat dilateImg;//是反二值化图片
	dilate(edges, dilateImg, element);
	imshow("边缘膨胀处理", dilateImg);

	//去除小像素点
	vector<vector<Point>> contours;//轮廓
	Mat removeDot = dilateImg.clone();
	findContours(dilateImg, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	for (uint32_t i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect((Mat)contours[i]);
		if (rect.height <= 15 && rect.width <= 15) {
			drawContours(removeDot, contours, i, Scalar(0), -1);//小像素点涂为黑色
		}
	}
	imshow("去除小像素点", removeDot);

	//垂直方向膨胀处理
	Mat element1 = getStructuringElement(MORPH_RECT, Size(1, 20));//膨胀处理的kernel,第三个参数不填表示默认为核的中心
	Mat dilateImg1;//也是反二值化图片
	dilate(removeDot, dilateImg1, element1);
	imshow("垂直方向膨胀处理", dilateImg1);

	//找出垂直方向膨胀处理后的每个字符
	Mat verticalImg = Mat::zeros(edges.rows, edges.cols, CV_8U);
	contours.clear();
	findContours(dilateImg1, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);//CV_CHAIN_APPROX_SIMPLE
	double ratio;
	for (uint32_t i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect((Mat)contours[i]);
		ratio = (double)rect.height / (double)rect.width;
		if (ratio>1 && (rect.width>15 || rect.height>15))//高宽比大于1且高和宽不能同时小于15（都小于15的是噪声）
			rectangle(verticalImg, rect, Scalar(255), -1);
	}
	imshow("标记每个字符", verticalImg);

	//水平方向膨胀处理，膨胀宽度稍微大一些影响也不大
	Mat element2 = getStructuringElement(MORPH_RECT, Size(60, 1));
	Mat dilateImg2;
	dilate(verticalImg, dilateImg2, element2);
	imshow("水平方向膨胀处理", dilateImg2);

	//找出大致的数字区域
	Mat horizontalImg = Mat::zeros(edges.rows, edges.cols, CV_8U);
	contours.clear();
	findContours(dilateImg2, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);//找出图片（黑背景白前景）膨胀后的轮廓

	Rect maxRect(0, 0, 0, 0);//面积为0的矩形
	for (uint32_t i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect((Mat)contours[i]);
		if (rect.area() >= maxRect.area()) {
			maxRect = rect;
		}

		rectangle(horizontalImg, rect, Scalar(255), 1);
	}
	imshow("水平方向膨胀处理后的字符区域", horizontalImg);

	if (maxRect.height <= 50 && maxRect.width <= 60)
		return;//no roi

	//得到反二值化的ROI图
	Mat roi_src;
	src(maxRect).copyTo(roi_src);
	imshow("从原图上裁剪出ROI", roi_src);
	Mat roi_gray;
	cvtColor(roi_src, roi_gray, CV_BGR2GRAY);
	Mat roi;
	threshold(roi_gray, roi, 0, 255, CV_THRESH_OTSU);
	imshow("roi做otsu二值化", roi);
	//判断roi是不是黑白图
	if (!is_black_n_white(roi)) {
		threshold(roi, roi, 100, 255, CV_THRESH_BINARY_INV);//反二值化
	}
	imshow("最终的反二值化roi", roi);
	//倾斜校正，判断字符是不是垂直的
	//判断字符是不是垂直的
	int angle = computeAngle(roi.clone());
	if (std::abs(angle) <= 4) {//倾角小于4度，不旋转，字符视为垂直
		cout << "字符是 垂直 的" << endl;
	}
	else//字符视为倾斜
	{
		isVertical = false;
		roi = rotateImage(roi, angle);
		cout << "字符是 倾斜 的" << endl;
	}
	imshow("倾斜校正", roi);

	//取消注释可以分割字符制作模板，模板图必须按顺序包含 0123456789. 十个字符
	//createTemplate(roi);
}

int main() {
	findROI(".\\template\\sample8.jpg");

	cout << recognize(".\\template\\sample8.jpg") << endl;

	waitKey(0);
	return 0;
}

