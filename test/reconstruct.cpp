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
	
	String video_name = argv[1];

	VideoCapture reader[16];
	
	int total_bitrate = 0;
	for(int tiles = 0; tiles < 16; tiles++){
		ostringstream tilename;
		tilename << video_name << tiles << "_enc.mkv";
		reader[tiles].open(tilename.str());
		if(!reader[tiles].isOpened()){
			cout << "failed to open video" << endl;
			return -1;
		}
	}
	
	Mat tiles[16];
	Mat concat[10];
	Mat end[5];
	Mat dst;
	int frame = 1;
	for(;;) {

		for(int i = 0; i < 16; i++) {
			reader[i] >> tiles[i];
		}
		if(tiles[0].empty()) break;

		// for 4x4 tiling
		// first row
		int i = 0;
		for(int row = 0; row < 4; row++){
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
		out << video_name << frame << "final.png"<<flush;
		imwrite(out.str(), dst);
	}

	ostringstream command;
	command << "ffmpeg -i " << video_name << "\%d" << "final.png" << " -r 60 -vcodec png " << video_name << "_final.mkv";
	system(command.str().c_str());

	cout << "total bitrate is: " << total_bitrate << endl;
	ostringstream cleanup;
	cleanup << "rm " << video_name << "*.png";
	system(cleanup.str().c_str());

	return 0;
}

