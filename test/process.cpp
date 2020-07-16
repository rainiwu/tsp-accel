#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <stdlib.h>
#include <iostream>


using namespace cv;
using namespace std; 

// tile video
int main(int argc, char** argv)
{
	// opening video
	VideoCapture source_video(argv[1]);
	if (!source_video.isOpened()) {
		cout << "could not open input video: " << argv[1] << endl;
		return -1;
	}

	
	Size t = Size((int) source_video.get(CAP_PROP_FRAME_WIDTH), 
			(int) source_video.get(CAP_PROP_FRAME_HEIGHT));

	
	String video_name = argv[2];
	Mat src, dst;
	

	// preprocessing
	int tile_cols = source_video.get(CAP_PROP_FRAME_WIDTH) / 4;
	int tile_rows = source_video.get(CAP_PROP_FRAME_HEIGHT) / 4;
	Mat tiles[16];
	Mat concat[5];
	Mat blank(tile_rows, tile_cols, CV_8UC3, Scalar(0,0,0));
	Mat end[10];


	int frame = 1;
	for(;;)
	{
		source_video >> src;
		if(src.empty()){
			break;
		}

		int max_row = 4;
		int max_col = 4;
		int num = 0;
		// where the tiling happens
		for(int row = 0; row < max_row; row++){
			for(int col = 0; col < max_col; col++){
				src(Rect(tile_cols*col, tile_rows*row, tile_cols, tile_rows)).copyTo(tiles[num]);
				num++;
			}
		}
		
		/** for 3x3 tiling 
		//first row
		hconcat(tiles[0], tiles[1], concat[0]);
		hconcat(concat[0], tiles[2], concat[0]);
		//second row
		hconcat(tiles[3], tiles[4], concat[1]);
		hconcat(concat[1], tiles[5], concat[1]);
		//third row
		hconcat(tiles[6], tiles[7], concat[2]);
		hconcat(concat[2], tiles[8], concat[2]);
		//all together
		vconcat(concat[0], concat[1], concat[3]);
		vconcat(concat[3], concat[2], dst);
		**/

		// select four tiles to keep
		for(int tile_num = 0; tile_num < max_row*max_col; tile_num++){
			if(!(tile_num == 5 || tile_num == 6 || tile_num == 9 || tile_num == 10))
				blank.copyTo(tiles[tile_num]);
		}

		// for 4x4 tiling
		// first row
		int i = 0;
		for(int row = 0; row < max_row; row++){
			for(int col = 0; col < 2; col++){
				hconcat(tiles[i], tiles[i+1], concat[col]);
				i+=2;
			}
			hconcat(concat[0], concat[1], end[row]);
		}
		vconcat(end[0], end[1], end[0]);
		vconcat(end[2], end[3], end[2]);
		vconcat(end[0], end[2], dst);

		
		cout << "\r" << "processing frame " << frame++ << flush;
		ostringstream out;
		out << video_name << frame << ".png"<<flush;
		imwrite(out.str(), dst);
	}
	return 0;
}
/**
void generate_visible_tiles(int min_degree, int max_degree, int x_fov, int y_fov, int* tile_list) {
	int max_width = 6144;
	int num = 0;
	//int max_height = 3072;
	for(int width = 0; width < max_width; width++){
	//	for(int height = 0; height < max_height; height++) {		
			for(int i = min_degree; i <= max_degree; i++) {
						
			}
	//	}
	}
}
**/
