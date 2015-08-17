/***********************************************************************************************************
 * Authors:  Allen Woods, Kal Stankov
 *
 * EGRE 526 Final Project:  UDP Video Streamer
 *
 * Description: Client program to initialize web camera and send frames through UDP sockets (using Windows API) to remote server
 *
 **********************************************************************************************************/

/*Additional library dependencies to add to project configuration: ws2_32.lib;opencv_calib3d2411.lib;opencv_calib3d2411d.lib;opencv_contrib2411.lib;opencv_contrib2411d.lib;opencv_core2411.lib;opencv_core2411d.lib;opencv_features2d2411.lib;opencv_features2d2411d.lib;opencv_flann2411.lib;opencv_flann2411d.lib;opencv_gpu2411.lib;opencv_gpu2411d.lib;opencv_highgui2411.lib;opencv_highgui2411d.lib;opencv_imgproc2411.lib;opencv_imgproc2411d.lib;opencv_legacy2411.lib;opencv_legacy2411d.lib;opencv_ml2411.lib;opencv_ml2411d.lib;opencv_nonfree2411.lib;opencv_nonfree2411d.lib;opencv_objdetect2411.lib;opencv_objdetect2411d.lib;opencv_ocl2411.lib;opencv_ocl2411d.lib;opencv_photo2411.lib;opencv_photo2411d.lib;opencv_stitching2411.lib;opencv_stitching2411d.lib;opencv_superres2411.lib;opencv_superres2411d.lib;opencv_ts2411.lib;opencv_ts2411d.lib;opencv_video2411.lib;opencv_video2411d.lib;opencv_videostab2411.lib;opencv_videostab2411d.lib;%(AdditionalDependencies)*/

#include "stdafx.h"
#include <sys\types.h>
#include <winsock2.h>

//OpenCV standard libraries
#include "opencv2/core/core.hpp"
#include "opencv2/flann/miniflann.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/photo/photo.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/core/core_c.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#define KEY_PRESS_DELAY 10
#define ESC_KEY_PRESSED	27	/*27 is ASCII value for the escape key*/
#define PORTNUMBER 2015

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

using namespace cv;	/*All OpenCV classes and functions are in the 'cv' namespace*/
using namespace std;

int main(void)
{
	VideoCapture vid_cap_1(0);	/*Open video camera 0*/

	if(!vid_cap_1.isOpened())		/*Error checking initialization of video camera*/
	{
		cout<<"ERROR: The camera could not be opened.  Program will now exit."<<endl;
		return -1;
	}	/*end if*/

	int err_check = 0;
	int frame_data_size = 0;
	int server_address_length;
	double frame_width = vid_cap_1.get(CV_CAP_PROP_FRAME_WIDTH);	/*Obtains width of video frames*/
	double frame_height = vid_cap_1.get(CV_CAP_PROP_FRAME_HEIGHT);	/*Obtains height of video frames*/
	Mat frame, continuous_frame, img1;



	struct sockaddr_in server_address;	/*sockaddr_in is a structure that contains Internet address information*/
	struct hostent *server;	/*'server' is a pointer to the hostent structure, which defines a host computer on the Internet (address information in struct members)*/
	
	/*Windows Socket declarations*/
	SOCKET socket_file_desc_1 = INVALID_SOCKET;
	WSADATA	wsaData;

	if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0)	/*Initializing Windows Sockets Version 2.2*/
	{
		cout<<"ERROR: Program could not initialize Windows Sockets. Program will now exit."<<endl;
		return -1;
	}	/*end if*/

	socket_file_desc_1 = socket(AF_INET, SOCK_DGRAM, 0);	/*Creating the socket: AF_INET for Internet address domain, SOCK_DGRAM for UDP socket type, and 0 to allow OS to choose transport layer protocol (UDP will be chosen because of second argument).*/ 
	if(socket_file_desc_1 < 0)
		printf("ERROR: Socket could not be opened.\n");

	server = gethostbyname("127.0.0.1");		/*Takes name of server and returns a pointer to a hostent structure containing information about the host*/
	if(server == NULL)
	{
		printf("ERROR: Receiver host not found.\n");
		return -1;
	}	/*end if*/

	memset(&server_address, 0, sizeof(server_address));	/*Initializing the server address buffer to 0*/

	/***Setting the fields in server_address***/
	server_address.sin_family = AF_INET;
	memmove(&server_address.sin_addr.s_addr,
		server->h_addr,		/*h_addr is defined as the first address in the array h_addr_list*/
		server->h_length);

	server_address.sin_port = htons(PORTNUMBER);		/*htons() converts a port number in host byte order to a port number in network byte order*/
	
	cout<<"Video frame size (W x H): "<<frame_width<<" x "<<frame_height<<endl;
	namedWindow("Sender_Window", CV_WINDOW_AUTOSIZE);	/*Creates window called "Sender_Window"*/

	
	
	while(true)
	{
		vid_cap_1.retrieve(frame);	/*Retrieves a new frame from the video stream*/

		if(vid_cap_1.read(frame) == false)	/*Reads the retrieved frame from the video stream; breaks out of loop if read failure occurs*/
		{
			cout<<"ERROR: Unable to read frame from video stream.  Program will now exit."<<endl;
			return -1;
		}	/*end if*/

		resize(frame, frame, Size(320, 200), 0, 0, INTER_CUBIC);
		
		imshow("Sender_Window", frame);	/*Sends read-in frame to "Sender_Window"*/

		
		flip(frame, frame, 1);
		continuous_frame = frame.reshape(0,1);	/*Makes the input frame continuous so that it can be transferred on the network*/

		//frame_data_size = (int) strlen((const char *) continuous_frame.data);
		frame_data_size = sizeof(continuous_frame.data);
		server_address_length = sizeof(server_address);

		
		err_check = sendto(socket_file_desc_1, (const char *) continuous_frame.data, 65507, 0, (struct sockaddr *) &server_address, server_address_length);	/*Sending frames on socket*/
		//err_check = sendto(socket_file_desc_1, (const char *) continuous_frame.data, frame_data_size, 0, (struct sockaddr *) &server_address, server_address_length);	/*Sending frames on socket*/
		
		if(err_check == SOCKET_ERROR)
			cout<<"ERROR: Failure sending on socket. Code "<<WSAGetLastError()<<endl;
		else
			cout<<"Sent frame of size "<<err_check<<" bytes"<<endl;



		if(waitKey(KEY_PRESS_DELAY) == ESC_KEY_PRESSED)	/*Wait for 'ESC' key to be pressed for KEY_PRESS_DELAY ms. HighGui requires regular waitKey() calls with at least 1ms delays to process windows events like redraw, resizing, input event etc. */
		{
			cout<<"User pressed ESC key."<<endl;
			break;
		}	/*end if*/
	}	/*end while*/

	return 0;
}

