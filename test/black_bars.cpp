#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "../tsp.hpp"

#define WIDTH 6144
#define HEIGHT 4608
#define FPS 60
#define TP 0.4
// set REVERT to 1 and recompile for TSP-->CMP conversion
#define REVERT 1

int main(void) {
	VideoCapture source_video("../media/balboa_cmp_trunc2.mkv");
	int ex = static_cast<int>(source_video.get(CAP_PROP_FOURCC));
	Size t = Size(WIDTH, HEIGHT);

}
