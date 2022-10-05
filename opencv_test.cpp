#include <opencv2/opencv.hpp>  
#include <opencv2/imgproc.hpp>
#include "opencv2/highgui.hpp"
#include <opencv2/imgcodecs.hpp>
#include "contour.cpp"

using namespace cv;
using namespace std;

const char* win_name = "demo";

struct bindata{
    Mat g_image;
    Mat b_image;
    int threshold_value;
    int open_kernel_size;
    int erosion_kernel_size;
};

void process(bindata data)
{
    ///////////二值化阈值/////////////
    /* 0: Binary
     1: Binary Inverted
     2: Threshold Truncated
     3: Threshold to Zero
     4: Threshold to Zero Inverted
    */
    threshold( data.g_image, data.b_image, data.threshold_value, 255, 1 );

    ////////////////开运算//////////////
    // (后来实验发现开运算其实没必要做）//
     //Mat element = getStructuringElement(MORPH_RECT,Size(data.open_kernel_size,data.open_kernel_size));
    // // //operation: 2(opening), 3(closing), 4(gradient), 5(top hat), 6(black hat)
     //morphologyEx(data.b_image,data.b_image,2,element);

    //////////////腐蚀/////////////////
    Mat element = getStructuringElement(MORPH_RECT,Size(data.erosion_kernel_size,data.erosion_kernel_size));
    erode(data.b_image,data.b_image,element);


    ///////////////////////找轮廓/////////////////////////////
    vector<vector<Point>> contours_;
    vector<Vec4i> hierarchy_;
    findContours(data.b_image,contours_,hierarchy_,RETR_TREE,CHAIN_APPROX_NONE);
    // drawContours(data.b_image,contours_,-1,(0,0,124),2,8,hierarchy_,2);
    vector<double> tmp;
    
    // 用opencv来做
    // for(int i=0;i<contours_.size();i++)
    // {
    //     tmp.push_back(contourArea(contours_[i]) );
    // }
    // sort(tmp.begin(),tmp.end());
    
    // cout << "opencv:" << contours_.size() << endl;
    // for(int i=0;i<contours_.size();i++)
    // {
    //     cout << tmp[i] << ' ';
    // }
    // cout << endl;
    // tmp.clear();

    // for (int i = 0; i<hierarchy_.size(); i++)
    // {
    //     if (hierarchy_[i][3] != -1)
    //     {
    //         tmp.push_back(contourArea(contours_[i]));
    //     }
    // }    

    // cout << "opencv sibling:"<<tmp.size()<<endl;
    // sort(tmp.begin(),tmp.end());
    // for(int i=0;i<tmp.size();i++)
    // {
    //     cout << tmp[i] << ' ';
    // }

    //从opencv接入非opencv
    int numrow = data.b_image.rows;
    int numcol = data.b_image.cols;

    vector<vector<int>> image(numrow);
    for(int r=0;r<numrow;++r)
    {
        for(int c=0;c<numcol;++c)
        {
            int tmp = data.b_image.at<uchar>(r, c);
            if(tmp!=0) tmp=1;//避免二值化不完全
            image[r].push_back(tmp);
        }
    }

    vector<contour::MyNode> hierarchy;
    vector<vector<contour::MyPoint>> contours;
    contour::find_contours(image,contours,hierarchy);
    //contour::printHierarchy(hierarchy);
    auto color = contour::createChannels(numrow,numcol,hierarchy,contours);

    //在另外一个窗口显示contour算法结果
     Mat our_contour=data.g_image.clone();
     for(int i=0;i<numrow;i++)
     {
         for(int j=0;j<numcol;j++)
         {
             auto c=color[i][j];
             our_contour.at<uchar>(i,j) = c.red;
         }
     }
     imshow("our contour",our_contour);

    //vector<double> tmp;
    vector<int> tmp_idx;//tmp中的序号在contour中对应的序号
    for(int i=1;i<hierarchy.size();i++)//idx start from 1
    {
        int parent = hierarchy[i].parent;
        if(parent !=1)
        {
            tmp.push_back(contour::contourArea(contours[i-1]));
            tmp_idx.push_back(i - 1);
        }
    }

    //输出计算得到的内轮廓面积大小
	cout << "ours sibling:" <<tmp.size()<<endl;
    sort(tmp.begin(),tmp.end());
    for(int i=0;i<tmp.size();i++)
    {
        cout << tmp[i] << ' ';
    }
    cout << endl;

    //根据上下阈值进行筛选
    int area_threshold = 1500,area_upper_bound = 4200;
	int res = 0;
    int size = tmp.size();
	for (int i = 0; i < size; i++)
	{
		if (tmp[i] > area_threshold && tmp[i] < area_upper_bound)
		{
			res++;
			int index = tmp_idx[i];
			int r, c;
            // 在当前窗口显示contour算法结果（以灰色显示螺母内轮廓）
			for (int j = 0; j < contours[index].size(); j++) {
				r = contours[index][j].row;
				c = contours[index][j].col;
                data.b_image.at<uchar>(r+2, c+2) = 126;
                data.b_image.at<uchar>(r+2, c+1) = 126;
                data.b_image.at<uchar>(r+1, c+2) = 126;

                data.b_image.at<uchar>(r+1, c+1) = 126;
                data.b_image.at<uchar>(r, c+1) = 126;
                data.b_image.at<uchar>(r+1, c) = 126;
                data.b_image.at<uchar>(r, c) = 126;
			}
		}
	}
    cout << "nut num: " << res;
    cout << endl << endl;
}


void proc_threshold( int threshold_value, void*p )
{
    bindata* p_ = (bindata *)p;
    p_->threshold_value = threshold_value;
    bindata data = *p_;
    process(data);
    imshow( win_name, data.b_image );
}

void proc_opening(int kernel_size ,void * p)
{
    bindata *p_ = (bindata*) p;
    p_->open_kernel_size = kernel_size;
    bindata data = *p_;
    process(data);
    imshow(win_name,data.b_image);
}

void proc_erosion(int kernel_size,void*p)
{
    bindata *p_ = (bindata*) p;
    p_->erosion_kernel_size = kernel_size;
    bindata data = *p_;
    process(data);
    imshow(win_name,data.b_image);
}


// https://docs.opencv.org/4.x/d1/dfb/intro.html
int main()
{
    char imageName[] = "test.bmp";
    Mat m_image = imread(imageName, IMREAD_COLOR);   // 读入图片 
    Mat g_image;//灰度图
    Mat b_image;//二值图

    if (m_image.empty())     // 判断文件是否正常打开  
    {
        fprintf(stderr, "Can not load image %s\n", imageName);
        waitKey(6000);  // 等待6000 ms后窗口自动关闭   
        return -1;
    }

    //imshow("image", m_image);  // 显示图片 
    // waitKey();
    // imwrite("test.jpg", m_image); // 存为bmp格式图片

    //开始处理
    cvtColor(m_image,g_image,COLOR_BGR2GRAY);
    b_image = g_image.clone();

    bindata data{g_image,b_image,1,1,1};
    namedWindow( win_name, WINDOW_AUTOSIZE ); // Create a window to display results
    
    //////////////////////选取二值化阈值//////////////////////
    int threshold_value=135;//经过下面的选取，140是个不错的值
    createTrackbar( "threshold_value",
                win_name, &threshold_value,
                255, proc_threshold,(void*)(&data)); // Create a Trackbar to choose Threshold value
    // proc_threshold(140,&data);

    //////////////////////开运算去掉白噪声/////////////////////
    //（后来发现开运算其实没必要做）///
    //int open_kernel_size=1;
    //createTrackbar("Opening kernel size",win_name,&open_kernel_size,21,proc_opening,(void*)(&data));
    
    //////////////////////腐蚀去掉粘连////////////////////////
    int erosion_kernel_size = 6;
    createTrackbar("erosion kernel size",win_name,&erosion_kernel_size,25,proc_erosion,(void*)(&data));




    waitKey();
    return 0;
}

