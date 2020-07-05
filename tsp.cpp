#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

// enum to describe location of face
//enum Face { LEFT, BASE, RIGHT, BOTTOM, BACK, TOP };
enum Face {TOP, LEFT, BASE, RIGHT, BACK, BOTTOM};

// passes by reference the requested transform matrix
void get_transform(Face face_type, int max_offset, int face_edge, Point2f* output_quad) {
	switch(face_type) {
		case BASE:
			output_quad[0] = Point2f( 0, 0 ); // top left
			output_quad[1] = Point2f( face_edge, 0 ); // top right
			output_quad[2] = Point2f( 0, face_edge ); // bottom left
			output_quad[3] = Point2f( face_edge, face_edge ); // bottom right
			break;
		case LEFT:
			output_quad[0] = Point2f( 2*max_offset, max_offset ); // tl
			output_quad[1] = Point2f( face_edge, 0 ); // tr
			output_quad[2] = Point2f( 2*max_offset, face_edge - max_offset ); // bl
			output_quad[3] = Point2f( face_edge , face_edge); // br
			break;
		case RIGHT:
			output_quad[0] = Point2f( 0, 0 ); // tl
			output_quad[1] = Point2f( face_edge-2*max_offset, max_offset ); // tr
			output_quad[2] = Point2f( 0, face_edge ); // bl
			output_quad[3] = Point2f( face_edge-2*max_offset, face_edge - max_offset ); // br
			break;
		case TOP:
			output_quad[0] = Point2f( max_offset, 2*max_offset ); // tl
			output_quad[1] = Point2f( face_edge - max_offset, 2*max_offset ); // tr
			output_quad[2] = Point2f( 0, face_edge ); // bl
			output_quad[3] = Point2f( face_edge, face_edge ); // br
			break;
		case BOTTOM:
			output_quad[0] = Point2f( 0, 0 ); // tl
			output_quad[1] = Point2f( face_edge, 0 ); // tr
			output_quad[2] = Point2f( max_offset, face_edge-2*max_offset ); // bl
			output_quad[3] = Point2f( face_edge - max_offset, face_edge-2*max_offset ); // br
			break;
		case BACK:
			output_quad[0] = Point2f( 0, max_offset ); // tl
			output_quad[1] = Point2f( face_edge - 2*max_offset, max_offset ); // tr
			output_quad[2] = Point2f( 0, face_edge - max_offset); // bl
			output_quad[3] = Point2f( face_edge-2*max_offset, face_edge - max_offset ); // br
			break;
	}
}

// function to form tsp for a specific face
// takes the face type, face image, and truncation parameter
// also takes a boolean revert flag to allow reversions when appropriate
void tspform(Face face_type, Mat& face_mat, float trunc_param, Mat& dest_mat, bool revert)
{
	// this line is independent of cols or rows used, as face guaranteed to be square
	int max_offset = (int) (0.5*(face_mat.rows)*(1 - trunc_param)); 
	// the maximum index of pixels in the face
	int face_edge = face_mat.cols; 

	// get the appropriate transform
	Point2f input_quad[4];
	Point2f output_quad[4];
	get_transform(face_type, max_offset, face_edge, output_quad);
	get_transform(BASE, max_offset, face_edge, input_quad);

	// warp the square image
	if(revert){
		warpPerspective(face_mat, dest_mat, getPerspectiveTransform(output_quad, input_quad), dest_mat.size());	
	} else {
		warpPerspective(face_mat, dest_mat, getPerspectiveTransform(input_quad, output_quad), dest_mat.size(), INTER_NEAREST, BORDER_REPLICATE);    	 	
	}
}

