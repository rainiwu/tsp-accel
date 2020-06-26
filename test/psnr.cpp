#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <stdlib.h>
#include <iostream>

#include "../tsp.hpp"

using namespace cv;
using namespace std;

double get_psnr(const Mat& I1, const Mat& I2)
{
	Mat s1;
	absdiff(I1, I2, s1);
	s1.convertTo(s1, CV_32F);
	s1 = s1.mul(s1);
	Scalar s = sum(s1);
	double sse = s.val[0] + s.val[1] + s.val[2];
	if (sse <= 1e-10)
		return 0;
	else 
	{
		double mse = sse / (double) (I1.channels() * I1.total());
		double psnr = 10.0 * log10((255*255) / mse);
		return psnr;
	}
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
	double psnr = 0.0;
	for(;;)
	{
		source_video >> src;
		ref_video >> ref;
		if(src.empty() || ref.empty()){
			cout << endl << "processed " << frame - 1 << " frames of video, psnr found to be:" << psnr / (frame-1)<< endl;
			break;
		}
		psnr = psnr + get_psnr(ref, src);
		cout << "\r" << "processing frame " << frame++ << flush;
		

	}
	return 0;
}
