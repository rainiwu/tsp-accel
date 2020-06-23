#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "../tsp.hpp"

#define WIDTH 6144
#define HEIGHT 4608
#define FPS 60
// set REVERT to 1 and recompile for TSP-->CMP conversion
#define REVERT 1
#define TP 0.1

#define ROW 1
#define COL 0
#define NUM 1

// enum Face {TOP, LEFT, BASE, RIGHT, BACK, BOTTOM};

using namespace cv;

int main(int argc, char** argv)
{
	VideoCapture source_video(argv[1]);
	int ex = static_cast<int>(source_video.get(CAP_PROP_FOURCC));
	Size t = Size(WIDTH, HEIGHT);
	
	int face_size = HEIGHT / 3;
	Mat src, dst;
	Mat face;

	//Mat map[2][3];
	Mat end_faces[6]; 
	
	source_video >> src;
	src(Rect(face_size*COL, face_size*ROW, face_size, face_size)).copyTo(face); // roi extraction
	tspform((Face) NUM, face, TP, dst, REVERT);

	if(REVERT) imwrite("revert.png", dst);
	else imwrite("output.png", dst);

	namedWindow( "Display Image", WINDOW_AUTOSIZE );
	imshow( "Display Image", dst); 

	waitKey(0);
	return 0;	

}
