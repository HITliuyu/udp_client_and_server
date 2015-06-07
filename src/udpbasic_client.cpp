/*
 * udpimage_client.cpp
 *
 *  Created on: May 29, 2015
 *      Author: liu
 */
#include <stdio.h>
#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <errno.h>
#include <arpa/inet.h>



using namespace cv;
using namespace std;


#define SERVER_PORT 8888
#define MAX_PKG_SIZE 20480

int k_times;                                           //define send k_times for each package

void error(const char *msg);

void udpc_requ(uchar* imagecopyData,int imgSize, int sockfd,const struct sockaddr_in addr,int len);

int main(int argc,char **argv)
{
/////////////////////////////////////UDP Initialization//////////////////////////////////
	int sockfd;
	struct sockaddr_in addr;

	if(argc!=2 && argc!= 3)
	{
		fprintf(stderr,"Usage:%s server_ip [image]\n",argv[0]);
		exit(1);
	}

	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd<0)
	{
		fprintf(stderr,"Socket Error:%s\n",strerror(errno));
		exit(1);
	}

	bzero(&addr,sizeof(struct sockaddr_in));
	addr.sin_family=AF_INET;
	addr.sin_port=htons(SERVER_PORT);
	if(inet_aton(argv[1],&addr.sin_addr)<0)
	{
		fprintf(stderr,"Ip error:%s\n",strerror(errno));
		exit(1);
	}

/////////////////////////////////////video initializaiton//////////////////////////////////
	 VideoCapture cap;
	 if(!argv[2]){
		 cap.open(0);                        // open webcam
	 }else{
		 cap.open(argv[2]);
	 }

	 if(!cap.isOpened())
	 {
		 return -1;
	 }


//	 cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);      // set width//
//	 cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);     // set height //

	 Mat image, imagecopy;
	 cap >> image;
	 if(! image.data )                              // Check for invalid input
	 {
		 cout <<  "Could not open or find the image" << std::endl ;
		 return -1;
	 }
	 int imgSize = image.total()*image.elemSize();        //calculate image size
	 k_times = imgSize / MAX_PKG_SIZE;                    //calculate k_times
	 cout<< k_times << endl;
	 printf("THe image rows is %d\n", image.rows);
	 printf("THe image cols is %d\n", image.cols);
	 printf("1.image size is %d\n",image.total()*image.elemSize() );

/////////////////////////////////////////////main loop//////////////////////////////////////////
	 while(1){

		 cap >> image;
		 if(! image.data )                              // Check for invalid input
		 {
			 cout <<  "Could not open or find the image" << std::endl ;
			 return -1;
		 }

		 printf("THe image rows is %d\n", image.rows);
		 printf("THe image cols is %d\n", image.cols);
		 printf("1.image size is %d\n",image.total()*image.elemSize() );
		 imagecopy = (image.reshape(0,1));               // to make it continuous, 3 channels and 1 row

		 udpc_requ(imagecopy.data, imgSize, sockfd, addr,sizeof(struct sockaddr_in));

		 imshow("client", image);
		 waitKey( 100);

	}
	close(sockfd);
}


void udpc_requ(uchar* imagecopyData,int imgSize, int sockfd,const struct sockaddr_in addr,int len)
{
	struct recvbuf {
		uchar buf[MAX_PKG_SIZE];
		int flag;
	};
	struct recvbuf sendData;
	int bytes = 0;

	for (int i = 0; i < k_times; i ++) {
		for (int j = 0; j < MAX_PKG_SIZE; j ++ ){
			sendData.buf[j] = imagecopyData[i*MAX_PKG_SIZE + j];
		}
		if(i == 44){
			sendData.flag = 2;
		}else{
			sendData.flag = 1;
		}
		if ((bytes = sendto(sockfd, &sendData , sizeof(sendData), 0,(struct sockaddr*) &addr, len)) == -1) {
					 error("1.send failed");
		}

	}
	printf("Send data finished!\n");

}


void error(const char *msg)
{
    perror(msg);
    exit(1);
}












