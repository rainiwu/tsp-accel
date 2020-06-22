#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "tsp.hpp"

#define WIDTH 6144
#define HEIGHT 4608
#define FPS 60
#define TP 0.4
// set REVERT to 1 and recompile for TSP-->CMP conversion
#define REVERT 1

using namespace cv;

// main functionality of tsp-accel: takes a CMP video and converts to TSP or vice versa.
int main(int argc, char** argv)
{
	VideoCapture source_video(argv[1]);
	int ex = static_cast<int>(source_video.get(CAP_PROP_FOURCC));
	// not used in this case as source video may be raw (without defined dimensions)
//	Size S = Size((int) source_video.get(CAP_PROP_FRAME_WIDTH),    // Acquire input size
//                  (int) source_video.get(CAP_PROP_FRAME_HEIGHT));
	Size t = Size(WIDTH, HEIGHT);
	VideoWriter output_video;
	String video_name;
	if(REVERT){
		video_name = "revert.avi";
	} else {
		video_name = "output.avi";
	}
	output_video.open(video_name, ex, FPS, t, true);
	// int face_size = source_video.get(CAP_PROP_FRAME_HEIGHT) / 3;
	int face_size = HEIGHT / 3;
	Mat src, dst;
	Mat face;

	// blank face for unpacked layout
	Mat blank(face_size, face_size, CV_8UC3, Scalar(0,0,0));
	//Mat map[2][3];
	Mat end_faces[6]; 
	//Mat concat[3];
	Mat concat[5];
	for(;;)
	{
		source_video >> src;
		if(src.empty()) break;
		
//		face = src(Rect(0,0, 200, 200));
		/** // loop for packed cubemap
		for(int k = 0; k < 2; k++){
			for(int j = 0; j < 3; j++){
				src(Rect(face_size*j,face_size*k, face_size, face_size)).copyTo(face);
				tspform( (Face) (j + (k*3)), face, 0.4, map[k][j]);
			}
		}
		// concantations of the map for 3x2 layout
		hconcat(map[0][0], map[0][1], concat[0]);
		hconcat(concat[0], map[0][2], concat[1]);
		hconcat(map[1][0], map[1][1], concat[0]);
		hconcat(concat[0], map[1][2], concat[2]);
		vconcat(concat[1], concat[2], dst); 
		**/
		// concats of the map for unpacked layout
			
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
