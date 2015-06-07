/*
 * udpimage_server.cpp
 *
 *  Created on: May 29, 2015
 *      Author: liu
 */
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <pthread.h>
#include <errno.h>

using namespace cv;
using namespace std;


#define SERVER_PORT 8888                               //Define the port
#define MAX_PKG_SIZE 20480                             //Define max package size

int k_times;                                           //send k_times for each package
Mat  image = Mat::zeros( 480, 640, CV_8UC3);           //Define image size

void error(const char *msg);
void *servershow(void *arg);
void udps_respon(Mat image, int imgSize,  int sockfd);

int main(void)
{
/////////////////////////////////UDP Initialization///////////////////////////////////

	int sockfd;
	struct sockaddr_in addr;


	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd<0)
	{
		fprintf(stderr,"Socket Error:%s\n",strerror(errno));
		exit(1);
	}


	bzero(&addr,sizeof(struct sockaddr_in));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(SERVER_PORT);


	if(bind(sockfd,(struct sockaddr *)&addr,sizeof(struct sockaddr_in))<0)
	{
		fprintf(stderr,"Bind Error:%s\n",strerror(errno));
		exit(1);
	}


/////////////////////////////////Image Initialization///////////////////////////////

	int  imgSize = image.total()*image.elemSize();                 //calculate image size
	cout<<"image size is "<< imgSize <<endl;

	k_times = imgSize / MAX_PKG_SIZE;
	cout << k_times << endl;

//////////////////////////////////multithread stuff////////////////////////////////

	pthread_t tid;												  //thread id
	int error;
	void *status;

	error = pthread_create(&tid, NULL, servershow, NULL);
	if(error){
		printf("pthread is not created...\n");
		return -1;
	}

//////////////////////////////////main thread loop////////////////////////////////
	while(1){


		udps_respon(image, imgSize, sockfd);

	}

	close(sockfd);
	pthread_join(tid, &status);
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}


void *servershow(void *arg){

	while(true){
		imshow( "Server", image );
		waitKey(10);
	}
	return (void *)0;
}



void udps_respon(Mat image, int imgSize, int sockfd)
{
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	uchar sockData[imgSize];                  //receive buffer

	struct recvbuf {
			uchar buf[MAX_PKG_SIZE];
			int flag;
		};
	struct recvbuf recvData;


	int bytes = 0;
	int count = 0;
	for (int i = 0; i < k_times; i ++) {
			if ((bytes = recvfrom(sockfd, &recvData, sizeof(recvData), 0,(struct sockaddr*) &addr, &addrlen)) == -1) {
					 error("1.receive failed");
			}
			count = count + recvData.flag;
			for(int j = 0; j < MAX_PKG_SIZE; j ++){
				sockData[i*MAX_PKG_SIZE + j] = recvData.buf[j];
			}

//only in this case we store data to image, otherwise we break
		if(recvData.flag == 2){
			if(count == 46){
				printf("Receive data finished!\n");

				int ptr=0;
				for (int i = 0;  i < image.rows; i++) {
					for (int j = 0; j < image.cols; j++) {
							   image.at<cv::Vec3b>(i,j) = cv::Vec3b(sockData[ptr+ 0],sockData[ptr+1],sockData[ptr+2]);
							   ptr=ptr+3;
					}
				}
				cout<< "Write back image data finished!" << endl;


			}else{
				break;
			}
		}
	}


}



