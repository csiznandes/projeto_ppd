#include "filters.h"
#include <omp.h>

using namespace cv;

//Guassiano
void gaussianFilter(Mat &frame){

    Mat copia = frame.clone();

    int kernel[3][3] = {
        {1,2,1},
        {2,4,2},
        {1,2,1}
    };

    #pragma omp parallel for  //OpenMP, para mostrar paralelização dos pixels, divididos por linhas
    for(int y=1; y<frame.rows-1; y++){  

        for(int x=1; x<frame.cols-1; x++){

            Vec3i soma(0,0,0);

            for(int ky=-1; ky<=1; ky++){

                for(int kx=-1; kx<=1; kx++){

                    Vec3b pixel=
                    copia.at<Vec3b>(y+ky,x+kx);

                    int peso=
                    kernel[ky+1][kx+1];

                    soma[0]+=pixel[0]*peso;
                    soma[1]+=pixel[1]*peso;
                    soma[2]+=pixel[2]*peso;
                }
            }

            frame.at<Vec3b>(y,x)[0]=saturate_cast<uchar>(soma[0]/16);
            frame.at<Vec3b>(y,x)[1]=saturate_cast<uchar>(soma[1]/16);
            frame.at<Vec3b>(y,x)[2]=saturate_cast<uchar>(soma[2]/16);
        }
    }
}


//Sharpen
void sharpenFilter(Mat &frame){

    Mat copia=frame.clone();

    int kernel[3][3]={
        {0,-1,0},
        {-1,5,-1},
        {0,-1,0}
    };

    #pragma omp parallel for  //OpenMP, para mostrar paralelização dos pixels, divididos por linhas
    for(int y=1;y<frame.rows-1;y++){

        for(int x=1;x<frame.cols-1;x++){

            Vec3i soma(0,0,0);

            for(int ky=-1;ky<=1;ky++){

                for(int kx=-1;kx<=1;kx++){

                    Vec3b pixel=
                    copia.at<Vec3b>(y+ky,x+kx);

                    int peso=
                    kernel[ky+1][kx+1];

                    soma[0]+=pixel[0]*peso;
                    soma[1]+=pixel[1]*peso;
                    soma[2]+=pixel[2]*peso;
                }
            }

            frame.at<Vec3b>(y,x)[0]=saturate_cast<uchar>(soma[0]);
            frame.at<Vec3b>(y,x)[1]=saturate_cast<uchar>(soma[1]);
            frame.at<Vec3b>(y,x)[2]=saturate_cast<uchar>(soma[2]);

        }
    }
}


//Sobel
void sobelFilter(Mat &frame){

    Mat copia=frame.clone();

    int gx[3][3]={
        {-1,0,1},
        {-2,0,2},
        {-1,0,1}
    };

    int gy[3][3]={
        {-1,-2,-1},
        {0,0,0},
        {1,2,1}
    };

    #pragma omp parallel for  //OpenMP, para mostrar paralelização dos pixels, divididos por linhas
    for(int y=1;y<frame.rows-1;y++){

        for(int x=1;x<frame.cols-1;x++){

            int somaX=0;
            int somaY=0;

            for(int ky=-1;ky<=1;ky++){

                for(int kx=-1;kx<=1;kx++){

                    Vec3b pixel=
                    copia.at<Vec3b>(y+ky,x+kx);

                    int gray=
                    (pixel[0]+pixel[1]+pixel[2])/3;

                    somaX+=gray*gx[ky+1][kx+1];
                    somaY+=gray*gy[ky+1][kx+1];

                }
            }

            int magnitude=
            sqrt((somaX*somaX)+(somaY*somaY));

            magnitude=
            saturate_cast<uchar>(magnitude);

            frame.at<Vec3b>(y,x)[0]=magnitude;
            frame.at<Vec3b>(y,x)[1]=magnitude;
            frame.at<Vec3b>(y,x)[2]=magnitude;
        }
    }
}