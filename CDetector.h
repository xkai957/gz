#pragma once
#include <darknet/yolo_v2_class.hpp>

namespace kmod 
{
//#define C_SHARP_MAX_OBJECTS 1000


	//struct bbox_t {
	//	unsigned int x, y, w, h;       // (x,y) - top-left corner, (w, h) - width & height of bounded box
	//	float prob;                    // confidence - probability that the object was found correctly
	//	unsigned int obj_id;           // class of object - from range [0, classes-1]
	//	unsigned int track_id;         // tracking id for video (0 - untracked, 1 - inf - tracked object)
	//	unsigned int frames_counter;   // counter of frames on which the object was detected
	//	float x_3d, y_3d, z_3d;        // center of object (in Meters) if ZED 3D Camera is used
	//};

	//struct image_t {
	//	int h;                        // height
	//	int w;                        // width
	//	int c;                        // number of chanels (3 - for RGB)
	//	float *data;                  // pointer to the image data
	//};

	//struct bbox_t_container {
	//	bbox_t candidates[C_SHARP_MAX_OBJECTS];
	//};

	//typedef int(*INIT) (const char *configurationFilename, const char *weightsFilename, int gpu);
	//typedef int(*DETECT_IMAGE)(const char *filename, bbox_t_container &container);
	//typedef int(*DETECT_MAT)(const unsigned* data, const size_t data_length, bbox_t_container &container);
	//typedef int(*DISPOSE)();


	class CDetectorX
	{
	public:
		CDetectorX();
		~CDetectorX();

		int  InitializeYolo(const char* configurationFilename, const char* weightsFilename, int gpu);
		int  Detect(const char* filename, bbox_t_container &container);
		int  Detect(const unsigned char* data, const size_t data_length, bbox_t_container &container);
		void Dispose();

	private:

		/*INIT			init;
		DETECT_IMAGE    detect_image;
		DETECT_MAT	    detect_mat;
		DISPOSE			dispose;*/

		bool			bvalid;
	};



}

