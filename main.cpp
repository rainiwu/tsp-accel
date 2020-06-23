#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <stdlib.h>
#include <iostream>
#include <ctime>

#include "tsp.hpp"

#define FPS 60
// set REVERT to 1 and recompile for TSP-->CMP conversion
#define REVERT 0
#define HELP "unexpected argument\nexpected: ./tsp <input_name> <TP>"

using namespace cv;
using namespace std;

// main functionality of tsp-accel: takes a CMP video and converts to TSP or vice versa.
int main(int argc, char** argv)
{
	// checking for valid arguments
	if(argc != 3){ 
		cout << HELP << endl; 
		return -1;
	}

	// checking for a valid TP value
	if(!(atof(argv[2]) > 0 && atof(argv[2]) < 1)) {
		cout << HELP << endl << "invalid TP value" << endl;
		return -1;
	}
	float TP = atof(argv[2]); 

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
	String video_name;
	if(REVERT){
		video_name = "revert.avi";
	} else {
		video_name = "output.avi";
	}

	cout << "creating " << video_name << endl;

	output_video.open(video_name, ex, FPS, t, true);
	int face_size = ((int) source_video.get(CAP_PROP_FRAME_HEIGHT)) / 3;
	Mat src, dst;
	Mat face;

	// blank face for unpacked layout
	Mat blank(face_size, face_size, CV_8UC3, Scalar(0,0,0));
	Mat end_faces[6]; 
	Mat concat[5];
	int frame = 1;
	clock_t time = clock();
	for(;;)
	{
		source_video >> src;
		if(src.empty()){
			time = clock() - time;
			cout << endl << "video processed in " << (float)time/CLOCKS_PER_SEC << " seconds" << endl;
			break;
		}
		
		cout << "\r" << "processing frame " << frame++ << flush;

		// extracting faces and feeding into tspform
		int num = 0;
		for(int row = 0; row < 3; row++){
			for(int col = 0; col < 4; col++){
				if(!((row == 0 || row == 2) && !(col == 1))){
					src(Rect(face_size*col, face_size*row, face_size, face_size)).copyTo(face);
					tspform((Face) num, face, TP, end_faces[num], REVERT);
					num++;
				}
			}
		}

		// concats of the map for unpacked layout, starting from top rwo down
		hconcat(blank, blank, concat[0]);
		hconcat(blank, end_faces[0], concat[1]);
		hconcat(concat[1], concat[0], concat[2]); // top row
		hconcat(end_faces[1], end_faces[2], concat[3]);
		hconcat(end_faces[3], end_faces[4], concat[4]);
		hconcat(concat[3], concat[4], concat[3]); // middle row
		hconcat(blank, end_faces[5], concat[4]);
		hconcat(concat[4], concat[0], concat[0]); // bottom row
		vconcat(concat[2], concat[3], concat[1]);
		vconcat(concat[1], concat[0], dst); //done
		
		output_video.write(dst);
	}
	return 0;
}
