#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <stdlib.h>
#include <iostream>

#include "tsp.hpp"

#define FPS 25
// PACKING to 0 for CMP, 1 for RCMP
// Note: RCMP only functions for TP = 1 at the moment
#define PACKING 0
// DEBUG changes 
#define DEBUG 0
#define HELP "unexpected argument\nexpected: ./tsp <input_name> <TP> <chunk_id> <revert>"

using namespace cv;
using namespace std;

// main functionality of tsp-accel: takes a CMP video and converts to TSP or vice versa.
int main(int argc, char** argv)
{
	// checking for valid arguments
	if(argc != 5){ 
		cout << HELP << endl; 
		return -1;
	}

	// checking for a valid TP value
	if(!(atof(argv[2]) > 0 && atof(argv[2]) <= 10)) {
		cout << HELP << endl << "invalid TP value" << endl;
		return -1;
	}
	float TP = atoi(argv[2])/10.0; 

	// chunk id argument
	int chunk_id = atoi(argv[3]);

	// revert bool
	int REVERT = atoi(argv[4]);

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
	if(!DEBUG){
		if(REVERT){
			video_name = "./tsp_rec/tsp"+to_string(chunk_id)+to_string(int(TP*10))+"0_rec.avi";
		} else {
			if (TP!=1) 
				video_name = "./tsp_all/tsp"+to_string(chunk_id)+to_string(int(TP*10))+"0.avi";
			else
				video_name = "./cmp_all/cmp"+to_string(chunk_id)+"0.avi";
		}
	} else { 
		if(!REVERT) video_name = "output.mkv";
		else video_name = "revert.mkv";
	}

	cout << "creating " << video_name << endl;

	output_video.open(video_name, ex, FPS, t, true);
	
	int face_size;
	switch(PACKING){
		case 0:
			face_size = ((int) source_video.get(CAP_PROP_FRAME_HEIGHT)) / 3;
			break;
		case 1:
			face_size = ((int) source_video.get(CAP_PROP_FRAME_HEIGHT)) / 2;
			break;
	}
	
	Mat src, dst;
	Mat face;
	
	// blank face for unpacked layout
	Mat blank(face_size, face_size, CV_8UC3, Scalar(0,0,0));

	Mat end_faces[6]; 
	Mat concat[5];
	int frame = 1;
	for(;;)
	{
		source_video >> src;
		if(src.empty()){
			cout << endl << "processed " << frame - 1 << " frames of " << face_size*4 << "x" 
				<< face_size*3 << " video at " << FPS << " fps" << endl;
			break;
		}
		
		cout << "\r" << "processing frame " << frame++ << flush;

		// extracting faces and feeding into tspform
		int num = 0;
		int max_row, max_col;
		switch(PACKING)
		{
			case 0:
				max_row = 3;
				max_col = 4;
				break;
			case 1:
				max_row = 2;
				max_col = 3;
		}

		for(int row = 0; row < max_row; row++){
			for(int col = 0; col < max_col; col++){
				if(!((row == 0 || row == 2) && !(col == 1)) || PACKING){
					src(Rect(face_size*col, face_size*row, face_size, face_size)).copyTo(face);
					tspform((Face) num, face, TP, end_faces[num], REVERT);
					num++;
				}
			}
		}

		switch(PACKING) {
			case 0:
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
			break;
			case 1:
			// concats of the map for packed layout, starting from top row down
			hconcat(end_faces[0], end_faces[1], concat[0]);
			hconcat(concat[0], end_faces[2], concat[0]);
			hconcat(end_faces[3], end_faces[4], concat[1]);
			hconcat(concat[1], end_faces[5], concat[1]);
			vconcat(concat[0], concat[1], dst);
			break;
		}
		output_video.write(dst);
	}
	return 0;
}
