#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#define WIDTH 6144
#define HEIGHT 4608
#define FPS 60
#define TP 0.4
// set REVERT to 1 and recompile for TSP-->CMP conversion
#define REVERT 0

using namespace cv;

// enum to describe location of face
//enum Face { LEFT, BASE, RIGHT, BOTTOM, BACK, TOP };
enum Face {TOP, LEFT, BASE, RIGHT, BACK, BOTTOM};

void tspform(Face face_type, Mat& face_mat, float trunc_param, Mat& dest_mat); 
void get_transform(Face face_type, int max_offset, int face_edge, Point2f* output_quad);

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
	int face_size = 1536;
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
					tspform((Face) num, face, TP, end_faces[num]);
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

/** debugging algorithim with signle image
int main(int argc, char** argv)
{
	Mat source;
	Mat dest;

	source = imread(argv[1], 1);
	float TP = 0.4;
	tspform( BACK, source, TP, dest );
	namedWindow( "Display Image", WINDOW_AUTOSIZE );
	imshow( "Display Image", dest); 

	waitKey(0);
	return 0;	
}
**/

// function to form tsp for a specific face
// takes the face type, face image, and truncation parameter
void tspform(Face face_type, Mat& face_mat, float trunc_param, Mat& dest_mat)
{
	// this line is independent of cols or rows used, as face guaranteed to be square
	int max_offset = (int) (0.5*(face_mat.rows)*(1 - trunc_param)); 
	// the maximum index of pixels in the face
	int face_edge = 1536;//face_mat.cols; 

	// get the appropriate transform
	Point2f input_quad[4];
	Point2f output_quad[4];
	get_transform(face_type, max_offset, face_edge, output_quad);
	get_transform(BASE, max_offset, face_edge, input_quad);

	// warp the square image
	if(REVERT){
		warpPerspective(face_mat, dest_mat, getPerspectiveTransform(output_quad, input_quad), dest_mat.size());	
	} else {
		warpPerspective(face_mat, dest_mat, getPerspectiveTransform(input_quad, output_quad), dest_mat.size());    	 	
	}
}

void get_transform(Face face_type, int max_offset, int face_edge, Point2f* output_quad) {
	switch(face_type) {
		case BASE:
			output_quad[0] = Point2f( 0, 0 ); // top left
			output_quad[1] = Point2f( face_edge, 0 ); // top right
			output_quad[2] = Point2f( 0, face_edge ); // bottom left
			output_quad[3] = Point2f( face_edge, face_edge ); // bottom right
			break;
		case LEFT:
			output_quad[0] = Point2f( 0, max_offset ); // tl
			output_quad[1] = Point2f( face_edge, 0 ); // tr
			output_quad[2] = Point2f( 0, face_edge - max_offset ); // bl
			output_quad[3] = Point2f( face_edge , face_edge); // br
			break;
		case RIGHT:
			output_quad[0] = Point2f( 0, 0 ); // tl
			output_quad[1] = Point2f( face_edge, max_offset ); // tr
			output_quad[2] = Point2f( 0, face_edge ); // bl
			output_quad[3] = Point2f( face_edge, face_edge - max_offset ); // br
			break;
		case TOP:
			output_quad[0] = Point2f( max_offset, 0 ); // tl
			output_quad[1] = Point2f( face_edge - max_offset, 0 ); // tr
			output_quad[2] = Point2f( 0, face_edge ); // bl
			output_quad[3] = Point2f( face_edge, face_edge ); // br
			break;
		case BOTTOM:
			output_quad[0] = Point2f( 0, 0 ); // tl
			output_quad[1] = Point2f( face_edge, 0 ); // tr
			output_quad[2] = Point2f( max_offset, face_edge ); // bl
			output_quad[3] = Point2f( face_edge - max_offset, face_edge ); // br
			break;
		case BACK:
			output_quad[0] = Point2f( 0, max_offset ); // tl
			output_quad[1] = Point2f( face_edge - 2*max_offset, max_offset ); // tr
			output_quad[2] = Point2f( 0, face_edge - max_offset); // bl
			output_quad[3] = Point2f( face_edge-2*max_offset, face_edge - max_offset ); // br
			break;
	}
}
