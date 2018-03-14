#include "sc.h"

using namespace cv;
using namespace std;


bool seam_carving(Mat& in_image, int new_width, int new_height, Mat& out_image){

    // some sanity checks
    // Check 1 -> new_width <= in_image.cols
    if(new_width>in_image.cols){
        cout<<"Invalid request!!! new_width has to be smaller than the current size!"<<endl;
        return false;
    }
    if(new_height>in_image.rows){
        cout<<"Invalid request!!! ne_height has to be smaller than the current size!"<<endl;
        return false;
    }
    
    if(new_width<=0){
        cout<<"Invalid request!!! new_width has to be positive!"<<endl;
        return false;

    }
    
    if(new_height<=0){
        cout<<"Invalid request!!! new_height has to be positive!"<<endl;
        return false;
        
    }

    //first-order Sobel derivatives
    Mat1f dx, dy, mag;

    Mat iimage = in_image.clone();
    Mat oimage = in_image.clone();

    //convert in_image to 8-bit grayscale 
    cvtColor(in_image, iimage, COLOR_RGB2GRAY);

    //compute Sobel derivatives and normalize from 8-bit to floating point
    Sobel(iimage, dx, CV_32F, 1, 0, 3, 1.0/255);
    Sobel(iimage, dy, CV_32F, 0, 1, 3, 1.0/255);
    
    //store gradient magnitude in out_image at each pixel
    magnitude(dx, dy, mag);

    Mat1f seamEnergy = Mat(in_image.rows, in_image.cols, CV_32F);
    int seamDirection[in_image.rows][in_image.cols];

    float min, left, up, right;
    int direction;
    for(int i=0; i < in_image.rows; i++) {
        for(int j=0; j < in_image.cols; j++) {
            if(i == 0) {
                seamEnergy.at<float>(i, j) = mag.at<float>(i, j);
                direction = 0;
            } else if(j == 0) {
                up = seamEnergy.at<float>(i-1, j);
                right = seamEnergy.at<float>(i-1, j+1);
                if(right < up) {
                    min = right;
                    direction = 1;
                } else {
                    min = up;
                    direction = 0;
                }
            } else if(j == in_image.cols - 1) {
                left = seamEnergy.at<float>(i-1, j-1);
                up = seamEnergy.at<float>(i-1, j);
                if(left < up) {
                    min = left;
                    direction = -1;
                } else {
                    min = up;
                    direction = 0;
                }
            } else {
                left = seamEnergy.at<float>(i-1, j-1);
                up = seamEnergy.at<float>(i-1, j);
                right = seamEnergy.at<float>(i-1, j+1);
                if(left < up) {
                    min = left;
                    direction = -1;
                } else {
                    min = up;
                    direction = 0;
                }
                if(right < min) {
                    min = right;
                    direction = 1;
                }
            }
            seamEnergy.at<float>(i, j) = mag.at<float>(i, j) + min;
            seamDirection[i][j] = direction;
        }
    }
    
    int minSeamIndex[in_image.rows];
    minSeamIndex[in_image.rows-1] = 0;
    for(int k=1; k < in_image.cols; k++) {
        if(seamEnergy.at<float>(in_image.rows-1, k) < seamEnergy.at<float>(in_image.rows-1, minSeamIndex[in_image.rows-1])) {
            minSeamIndex[in_image.rows-1] = k;
        }
    }

    for(int i=oimage.rows-1; i > 0; --i) {
        oimage.at<Vec3b>(i, minSeamIndex[i])[0] = 0;
        oimage.at<Vec3b>(i, minSeamIndex[i])[1] = 0;
        oimage.at<Vec3b>(i, minSeamIndex[i])[2] = 255;
        minSeamIndex[i-1] = minSeamIndex[i] + seamDirection[i][minSeamIndex[i]];
    }

    out_image = oimage;
    return true;
    
    //return seam_carving_trivial(in_image, new_width, new_height, out_image);
}


// seam carves by removing trivial seams
bool seam_carving_trivial(Mat& in_image, int new_width, int new_height, Mat& out_image){

    Mat iimage = in_image.clone();
    Mat oimage = in_image.clone();
    while(iimage.rows!=new_height || iimage.cols!=new_width){
        // horizontal seam if needed
        if(iimage.rows>new_height){
            reduce_horizontal_seam_trivial(iimage, oimage);
            iimage = oimage.clone();
        }
        
        if(iimage.cols>new_width){
            reduce_vertical_seam_trivial(iimage, oimage);
            iimage = oimage.clone();
        }
    }
    
    out_image = oimage.clone();
    return true;
}

// horizontal trivial seam is a seam through the center of the image
bool reduce_horizontal_seam_trivial(Mat& in_image, Mat& out_image){

    // retrieve the dimensions of the new image
    int rows = in_image.rows-1;
    int cols = in_image.cols;
    
    // create an image slighly smaller
    out_image = Mat(rows, cols, CV_8UC3);
    
    //populate the image
    int middle = in_image.rows / 2;
    
    for(int i=0;i<=middle;++i)
        for(int j=0;j<cols;++j){
            Vec3b pixel = in_image.at<Vec3b>(i, j);
            
            /* at operator is r/w
            pixel[0] = 255;
            pixel[1] =255;
            pixel[2]=255;
            */
            
            
            
            out_image.at<Vec3b>(i,j) = pixel;
        }
    
    for(int i=middle+1;i<rows;++i)
        for(int j=0;j<cols;++j){
            Vec3b pixel = in_image.at<Vec3b>(i+1, j);
            
            /* at operator is r/w
             pixel[0] --> red
             pixel[1] --> green
             pixel[2] --> blue
             */
            
            
            out_image.at<Vec3b>(i,j) = pixel;
        }

    return true;
}

bool reduce_vertical_seam_trivial(Mat& in_image, Mat& out_image){
    // retrieve the dimensions of the new image
    int rows = in_image.rows;
    int cols = in_image.cols-1;
    
    // create an image slighly smaller
    out_image = Mat(rows, cols, CV_8UC3);
    
    //populate the image
    int middle = in_image.cols / 2;
    
    for(int i=0;i<rows;++i)
        for(int j=0;j<=middle;++j){
            Vec3b pixel = in_image.at<Vec3b>(i, j);
            
            /* at operator is r/w
             pixel[0] --> red
             pixel[1] --> green
             pixel[2] --> blue
             */
            
            
            out_image.at<Vec3b>(i,j) = pixel;
        }
    
    for(int i=0;i<rows;++i)
        for(int j=middle+1;j<cols;++j){
            Vec3b pixel = in_image.at<Vec3b>(i, j+1);
            
            /* at operator is r/w
             pixel[0] --> red
             pixel[1] --> green
             pixel[2] --> blue
             */
            
            
            out_image.at<Vec3b>(i,j) = pixel;
        }
    
    return true;
}

bool reduce_vertical_seam(Mat& in_image, Mat& out_image){
    //create an image slighly smaller
    out_image = Mat(in_image.rows, in_image.cols-1, CV_8UC3);

    //convert in_image to 8-bit grayscale
    Mat iimage = in_image.clone(); 
    cvtColor(in_image, iimage, COLOR_RGB2GRAY);

    //first-order Sobel derivatives
    Mat1f dx, dy, mag;

    //compute Sobel derivatives and normalize from 8-bit to floating point
    Sobel(iimage, dx, CV_32F, 1, 0, 3, 1.0/255);
    Sobel(iimage, dy, CV_32F, 0, 1, 3, 1.0/255);
    
    //store gradient magnitude in out_image at each pixel
    magnitude(dx, dy, mag);

    //store cumulative minimum seam energy at each pixel, and seam direction pointing up
    Mat1f seamEnergy = Mat(in_image.rows, in_image.cols, CV_32F);
    int seamDirection[in_image.rows][in_image.cols];

    float min, left, up, right;
    int direction;

    //populate seamEnergy and seamDirection with dynamic programming algorithm
    for(int i=0; i < in_image.rows; i++) {
        for(int j=0; j < in_image.cols; j++) {
            if(i == 0) {
                seamEnergy.at<float>(i, j) = mag.at<float>(i, j);
                direction = 0;
            } else if(j == 0) {
                up = seamEnergy.at<float>(i-1, j);
                right = seamEnergy.at<float>(i-1, j+1);
                if(right < up) {
                    min = right;
                    direction = 1;
                } else {
                    min = up;
                    direction = 0;
                }
            } else if(j == in_image.cols - 1) {
                left = seamEnergy.at<float>(i-1, j-1);
                up = seamEnergy.at<float>(i-1, j);
                if(left < up) {
                    min = left;
                    direction = -1;
                } else {
                    min = up;
                    direction = 0;
                }
            } else {
                left = seamEnergy.at<float>(i-1, j-1);
                up = seamEnergy.at<float>(i-1, j);
                right = seamEnergy.at<float>(i-1, j+1);
                if(left < up) {
                    min = left;
                    direction = -1;
                } else {
                    min = up;
                    direction = 0;
                }
                if(right < min) {
                    min = right;
                    direction = 1;
                }
            }
            seamEnergy.at<float>(i, j) = mag.at<float>(i, j) + min;
            seamDirection[i][j] = direction;
        }
    }
    
    //store the pixel column of the minimum seam at each row  
    int minSeamColumn[in_image.rows];
    minSeamColumn[in_image.rows-1] = 0;

    //find min seam starting point from bottom of image
    for(int k=1; k < in_image.cols; k++) {
        if(seamEnergy.at<float>(in_image.rows-1, k) < seamEnergy.at<float>(in_image.rows-1, minSeamColumn[in_image.rows-1])) {
            minSeamColumn[in_image.rows-1] = k;
        }
    }

    //copy pixels into out_image, avoiding the seam
    for(int i=0; i < in_image.rows; i++) {
        for(int j=0; j < in_image.cols; j++) {
            if(j != minSeamColumn[i]) {
                out_image.at<Vec3b>(i, j) = in_image.at<Vec3b>(i, j);
            }
        }
    }

    return true;
}