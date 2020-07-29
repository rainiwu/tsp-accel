#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <stdlib.h>
#include <iostream>

#define IMAGE_WIDTH 6144
#define IMAGE_HEIGHT 3072
#define OFFSET 6

using namespace cv;
using namespace std; 

void translate_xy_to_spherical(int x, int y, double* a);

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
				ostringstream out;
				out << video_name << num << "-" << frame << ".png"<<flush;
				imwrite(out.str(), tiles[num]);
				num++;

			}
		}
		cout << "\r" << "processing frame " << frame++ << flush;
	}

	double coord[2];
	for(int i = 0; i < 16; i++) {
		ostringstream command;
		command << "ffmpeg -i " << video_name << i << "-" << "\%d" << ".png" << " -r 60 -vcodec png " << video_name << i << ".mkv";
		system(command.str().c_str());
	}

	int x, y;
	int total_weighted_tiles = 0;

	int c1_idx = 0;
	int c1[16];
	int c2_idx = 0;
	int c2[16];
	int c3_idx = 0;
	int c3[16];
	bool near_vp = false;
	for(int j = 0; j < 4; j++) 
		for(int k = 0; k < 4; k++){
			x = j*tile_cols;
			y = k*tile_rows;
			near_vp = false;
			while(x < (j+1)*tile_cols)
			{
				translate_xy_to_spherical(x, y, coord);
				if((coord[0] > (-45.0 + OFFSET) && coord[0] < (45.0 + OFFSET)) && (coord[1] > -45.0 && coord[1] < 45.0)){
					cout << "the tile at " << j << "x" << k << " is in vp" << " because of pixel " << x << "x" << y << endl;
					total_weighted_tiles += 3;
					c1[c1_idx] = (j*4) + k;
					c1_idx++;
					break;
				}
				else if((coord[0] > (-50.0 + OFFSET) && coord[0] < (50.0+ OFFSET)) && (coord[1] > -50.0 && coord[1] < 50.0)){	
					near_vp = true;
				}
				if(y < (k+1)*tile_rows - 1)
					y++;
				else {
					x++;
					y = k*tile_rows;
				}
				if(x >= (j+1)*tile_cols){
					if(near_vp) {
						cout << "the tile at " << j << "x" << k << " is near vp" << endl;
						total_weighted_tiles += 2;
						c2[c2_idx] = (j*4) + k;
						c2_idx++;
						break;
					} else if(!near_vp) {
						cout << "the tile at " << j << "x" << k << " is not near vp" << endl;
						total_weighted_tiles += 1;
						c3[c3_idx] = (j*4) +k;
						c3_idx++;
						break;
					}
				}
			}
		}

	double solo_bitrate = 70 / total_weighted_tiles;
	for(int a = 0; a < c1_idx; a++) {
		ostringstream encode;
		encode << "ffmpeg -i " << video_name << c1[a] << ".mkv " << "-c:v libx265 -b:v "<< 3*solo_bitrate <<"M -x265-params pass=1 -f null /dev/null"
			<< " && ffmpeg -i " << video_name << c1[a] << ".mkv " << "-c:v libx265 -b:v " << 3*solo_bitrate << "M -x265-params pass=2 "
			<< video_name << c1[a] << "_enc.mkv";
		system(encode.str().c_str());
	}
	for(int b = 0; b < c2_idx; b++) {
		ostringstream encode;
		encode << "ffmpeg -i " << video_name << c2[b] << ".mkv " << "-c:v libx265 -b:v "<< 2*solo_bitrate <<"M -x265-params pass=1 -f null /dev/null"
			<< " && ffmpeg -i " << video_name << c2[b] << ".mkv " << "-c:v libx265 -b:v " << 2*solo_bitrate << "M -x265-params pass=2 "
			<< video_name << c2[b] << "_enc.mkv";
		system(encode.str().c_str());

	}
	for(int c = 0; c < c3_idx; c++) {
		ostringstream encode;
		encode << "ffmpeg -i " << video_name << c3[c] << ".mkv " << "-c:v libx265 -b:v "<< solo_bitrate <<"M -x265-params pass=1 -f null /dev/null"
			<< " && ffmpeg -i " << video_name << c3[c] << ".mkv " << "-c:v libx265 -b:v " << solo_bitrate << "M -x265-params pass=2 "
			<< video_name << c3[c] << "_enc.mkv";
		system(encode.str().c_str());

	}

	cout << endl;
	ostringstream cleanup;
	cleanup << "rm " << video_name << "*.png";
	system(cleanup.str().c_str());

	return 0;
}

void translate_xy_to_spherical(int x, int y, double* answer) {
	answer[0] = (x / (IMAGE_WIDTH /360.0)) - 180.0;
	answer[1] = ((y / (IMAGE_HEIGHT / 180.0)) - 90.0) / -1;
}
