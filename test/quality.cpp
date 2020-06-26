#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <stdlib.h>
#include <iostream>

#include "../tsp.hpp"

using namespace cv;
using namespace std;

// test to determine the psnr loss of opencv
int main(int argc, char** argv)
{
	// opening video
	VideoCapture source_video(argv[1]);
	if (!source_video.isOpened()) {
		cout << "could not open input video: " << argv[1] << endl;
		return -1;
	}

	int ex = static_cast<int>(source_video.get(CAP_PROP_FOURCC));
	
	Size t = Size((int) source_video.get(CAP_PROP_FRAME_WIDTH), 
			(int) source_video.get(CAP_PROP_FRAME_HEIGHT));

	VideoWriter output_video;

	output_video.open("ran.mkv", ex, source_video.get(CAP_PROP_FPS), t, true);
	Mat src;

	int frame = 1;
	for(;;)
	{
		source_video >> src;
		if(src.empty()){
			cout << endl << "processed " << frame - 1 << " frames of video at " << source_video.get(CAP_PROP_FPS) << " fps" << endl;
			break;
		}
		
		cout << "\r" << "processing frame " << frame++ << flush;

		output_video.write(src);
	}
	return 0;
}
