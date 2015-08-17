/***********************************************************************************************************
 * Authors:  Allen Woods, Kal Stankov
 *
 * EGRE 526 Final Project:  UDP Video Streamer
 *
 * Description: Server program to receive client's web camera frames through UDP sockets (using Windows API)
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
#define UDP_PACKET_MAX_SIZE 65507 /*65,507 bytes = (65,535-byte data field) − (8-byte UDP header) − (20-byte IPv4 header)*/
#define ESC_KEY_PRESSED	27	/*27 is ASCII value for the escape key*/
#define PORTNUMBER 2015


// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

using namespace cv;	/*All OpenCV classes and functions are in the 'cv' namespace*/
using namespace std;


int main(void)
{
	Mat rec_frame, reshaped_frame;

	int i = 0, j = 0, k = 0, m = 0, err_check = 0, client_addr_length = 0, width = 320, height = 200;
	struct sockaddr_in server_address, client_address;		/*sockaddr_in (defined in winsock2.h) is a structure that contains Internet address information*/
	int iResult;
	u_long iMode = 0;
	

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

	iResult = ioctlsocket(socket_file_desc_1, FIONBIO, &iMode);	/*Makes the socket non-blockable so that recvfrom() continues the program if nothing has been read from the socket*/
	if (iResult != NO_ERROR)
	cout<<"ERROR: ioctlsocket failed with error: "<<iResult<<endl;

	memset(&server_address, 0, sizeof(server_address));	/*Initializing the server address buffer to 0*/


	/***Setting the fields in server_address***/
	/*The sockaddr_in structure specifies the address family, IP address, and port for the socket that is being bound. */
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORTNUMBER);		/*htons() converts a port number in host byte order to a port number in network byte order*/

	


	/***Setting up connection to client***/
	client_addr_length = sizeof(struct sockaddr_in);
	err_check = bind(socket_file_desc_1, (struct sockaddr *) &server_address, sizeof(server_address));		/*Binds socket to server IP address and port*/
	if(err_check == SOCKET_ERROR)		
		cout<<"ERROR: Failure on binding: Code "<<WSAGetLastError()<<endl;
	


	namedWindow("Receiver_Window", CV_WINDOW_AUTOSIZE);	/*Creates window called "Receiver_Window"*/
	rec_frame = Mat::zeros(height, width, CV_8UC1);
	//size_t rec_frame_size = rec_frame.total() * rec_frame.elemSize();	/*Setting frame size*/
	//int rec_frame_size = sizeof(rec_frame);
	//char *rec_buffer = new char[rec_frame_size];
	char *rec_buffer = new char[UDP_PACKET_MAX_SIZE];
	int rec_frame_size = UDP_PACKET_MAX_SIZE;
	//vector<uchar> videoBuffer;

	while(true)
	{
		//for(i = 0; i < rec_frame_size; i = i + err_check)
		//{
			err_check = recvfrom(socket_file_desc_1, rec_buffer, rec_frame_size, 0, (struct sockaddr *) &client_address, &client_addr_length);	/*Receiving information from the client on the socket. recvfrom() will return the number of bytes received.*/
			//err_check = recvfrom(socket_file_desc_1, rec_buffer + i, rec_frame_size - i, 0, (struct sockaddr *) &client_address, &client_addr_length);	/*Receiving information from the client on the socket. recvfrom() will return the number of bytes received and i provides a byte offset for reading.*/
			if (err_check == SOCKET_ERROR)
			{
				cout<<"Error receiving on socket. Code "<<WSAGetLastError()<<endl;
			}	/* end if*/
		//	else
			//	cout<<"Received frame of size "<<err_check<<" bytes"<<endl;

	
			for(i = 0; i < rec_frame.rows; i++)
			{
				//cout<<"Converting"<<endl;
				for(j = 0; j < rec_frame.cols; j++)
				{
					(rec_frame.row(i)).col(j) = (uchar) rec_buffer[(rec_frame.cols * i) + j];	/*Fills Mat-type matrix (row-major)*/
					//cout<<"i="<<i<<" j="<<j<<endl;
				}	/*end for*/
			}	/*end for*/

		memset(rec_buffer, 0, sizeof(rec_buffer));
		imshow("Receiver_Window", rec_frame);
			

			if(waitKey(KEY_PRESS_DELAY) == ESC_KEY_PRESSED)	/*Wait for 'ESC' key to be pressed for KEY_PRESS_DELAY ms. HighGui requires regular waitKey() calls with at least 1ms delays to process windows events like redraw, resizing, input event etc. */
			{
				cout<<"User pressed ESC key."<<endl;
				return 0;
			}	/*end if*/
		//}	/*end for*/


		
		/***Converting received data to the Mat class format*/
		
		//for(i = 0; i < rec_frame.rows; i++)
		//{
		//	cout<<"Gone Convertin'"<<endl;
		//	for(j = 0; j < rec_frame.cols; j++)
		//	{
		//		(rec_frame.row(i)).col(j) = (uchar) rec_buffer[(rec_frame.cols * i) + j];	/*Fills Mat-type matrix (row-major)*/
		//	}	/*end for*/
		//}	/*end for*/

		//memset(rec_buffer, 0, sizeof(rec_buffer));
		//imshow("Receiver_Window", rec_frame);

		if(waitKey(KEY_PRESS_DELAY) == ESC_KEY_PRESSED)	/*Wait for 'ESC' key to be pressed for KEY_PRESS_DELAY ms. HighGui requires regular waitKey() calls with at least 1ms delays to process windows events like redraw, resizing, input event etc. */
		{
			cout<<"User pressed ESC key."<<endl;
			break;
		}	/*end if*/

	}	/*end while*/



	return 0;
}	/*end main*/
