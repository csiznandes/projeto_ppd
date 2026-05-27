#include <iostream>
#include <mpi.h>
#include <opencv2/opencv.hpp>
#include "filters.h"

using namespace std;
using namespace cv;

int main(int argc,char** argv){

MPI_Init(&argc,&argv);  //Inicia o MPI

int rank,size;

MPI_Comm_rank(MPI_COMM_WORLD,&rank);
MPI_Comm_size(MPI_COMM_WORLD,&size);

//Coordenador
if(rank==0){

    VideoCapture video("video.mp4");

    if(!video.isOpened()){

        cout<<"Erro ao abrir video"<<endl;
        MPI_Finalize();
        return -1;
    }

    Mat frame;

    while(video.read(frame)){

        int rows=frame.rows;
        int cols=frame.cols;
        int type=frame.type();

        MPI_Send(&rows,1,MPI_INT,1,0,MPI_COMM_WORLD);
        MPI_Send(&cols,1,MPI_INT,1,0,MPI_COMM_WORLD);
        MPI_Send(&type,1,MPI_INT,1,0,MPI_COMM_WORLD);

        MPI_Send(
        frame.data,
        rows*cols*frame.elemSize(),
        MPI_UNSIGNED_CHAR,
        1,
        0,
        MPI_COMM_WORLD
        );

        cout<<"[COORDENADOR] Frame enviado"<<endl;

        MPI_Recv(
        frame.data,
        rows*cols*frame.elemSize(),
        MPI_UNSIGNED_CHAR,
        1,
        0,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE
        );

        cout<<"[COORDENADOR] Frame recebido"<<endl;

        imshow("Video Processado",frame);

        if(waitKey(30)==27)
            break;

        if(getWindowProperty("Video Processado", WND_PROP_VISIBLE) < 1)
            break;
    }

    int stop=-1;

    MPI_Send(
    &stop,
    1,
    MPI_INT,
    1,
    0,
    MPI_COMM_WORLD
    );
}

//Trabalhador
else{

while(true){

int rows;

MPI_Recv(
&rows,
1,
MPI_INT,
0,
0,
MPI_COMM_WORLD,
MPI_STATUS_IGNORE
);

if(rows==-1)
break;

int cols,type;

MPI_Recv(&cols,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
MPI_Recv(&type,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

Mat frame(rows,cols,type);

MPI_Recv(
frame.data,
rows*cols*frame.elemSize(),
MPI_UNSIGNED_CHAR,
0,
0,
MPI_COMM_WORLD,
MPI_STATUS_IGNORE
);

cout<<"[TRABALHADOR] Processando frame.."<<endl;

//ESCOLHER O FILTRO

//gaussianFilter(frame);
//sharpenFilter(frame);
sobelFilter(frame);

MPI_Send(
frame.data,
rows*cols*frame.elemSize(),
MPI_UNSIGNED_CHAR,
0,
0,
MPI_COMM_WORLD
);

cout<<"[TRABALHADOR] Frame processado"<<endl;

}

}

MPI_Finalize();

return 0;

}