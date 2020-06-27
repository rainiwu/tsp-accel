#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <stdlib.h>
#include <iostream>

#include "../tsp.hpp"

using namespace cv;
using namespace std;

Mat get_diff(const Mat& I1, const Mat& I2)
{
	Mat s1;
	Mat diff;
	vector<Mat> bgr;
	absdiff(I1, I2, s1);
	split(s1, bgr);
	//cout << typeid(diff).name() << endl;
	//cout << typeid(bgr[0]).name() << endl;
	diff = bgr[0] + bgr[1] + bgr[2];
	//cout << typeid(diff).name() << endl;
	diff.convertTo(diff, CV_32F);
	return diff;
}

// test to determine the psnr loss of opencv
int main(int argc, char** argv)
{
	// opening video
	VideoCapture ref_video(argv[1]);
	if (!ref_video.isOpened()) {
		cout << "could not open input video: " << argv[1] << endl;
		return -1;
	}

	VideoCapture source_video(argv[2]);
	if (!source_video.isOpened()) {
		cout << "could not open input video: " << argv[2] << endl;
		return -1;
	}

	Mat ref, src;

	int frame = 1;
	//double psnr = 0.0;

	Mat diff_sum = Mat(4608, 6144, CV_32F, double(0));
	Mat diff;
	Mat heatmap_color;
	double min, max;
	
	for(;;)
	{
		source_video >> src;
		ref_video >> ref;
		if(src.empty() || ref.empty()){
			//cout << endl << "processed " << frame - 1 << " frames of video, psnr found to be:" << psnr / (frame-1)<< endl;
			break;
		}
		//psnr = psnr + get_psnr(ref, src);
		diff = get_diff(ref, src);
		//cout << typeid(diff_sum).name() << endl;
		//cout << typeid(diff).name() << endl;
		diff_sum = diff_sum + diff;
		cout << "\r" << "processing frame " << frame++ << flush;
		

	}
	diff_sum = diff_sum/frame;
	namedWindow("heatmap",0);
	cvResizeWindow("heatmap", 1200, 900);
	//string widname = "heatmap";
	Mat heatmap_gray;
	normalize(diff_sum, heatmap_gray, 0, 255, NORM_MINMAX, CV_8UC1);
	minMaxLoc(diff_sum, &min, &max);
	cout << "min "<< min << "max" << max << endl;

	applyColorMap(heatmap_gray, heatmap_color, COLORMAP_JET);
	imwrite("../figures/psnr2.png", heatmap_color);
	imshow("heatmap", heatmap_color);
	waitKey(0);
	return 0;
}
