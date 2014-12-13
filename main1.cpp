#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <list>
using namespace cv;
using namespace std;

string hehe[100]={"time_0.jpg","time_1.jpg","time_2.jpg","time_3.jpg","time_4.jpg",
	"time_5.jpg","time_6.jpg","time_7.jpg","time_8.jpg","time_9.jpg",};

//-----------------------------------------------------------------------------
//平面几何相关函数http://www.cnblogs.com/zjutlitao/p/3243883.html
//-----------------------------------------------------------------------------
#define eps 0.0000000001
#define PI acos(-1.0)
int dcmp(double x){
    if(fabs(x)<eps)return 0;
    else return x<0 ? -1:1;
}
double Dot(Point A,Point B){return A.x*B.x+A.y*B.y;}//向量点积
double Length(Point A){return sqrt(Dot(A,A));}//向量模长
double Cross(Point A,Point B){return A.x*B.y-A.y*B.x;}//向量叉积
double Angle(Point A,Point B){return acos(Dot(A,B)/Length(A)/Length(B));}//求向量的夹角
double DistanceToLine(Point P,Point A,Point B)//点到直线的距离
{
	Point v1=B-A,v2=P-A;
	return fabs(Cross(v1,v2))/Length(v1);//如果不加绝对值是带有方向的距离
}
double DistancetoSegment(Point P,Point A,Point B){//点到线段的距离
    if(A==B)return Length(P-A);
    Point v1=B-A,v2=P-A,v3=P-B;
    if(dcmp(Dot(v1,v2))<0)return  Length(v2);
    else if(dcmp(Dot(v1,v3))>0)return Length(v3);
    else return fabs(Cross(v1,v2))/Length(v1);
}
//-----------------------------------------------------------------------------
class MyLine{
public:
	int id;//编号
	int k;//倾斜角[0-360)
	int l;//长度
public:
	MyLine(int ID=0,int K=0,int L=0){id=ID,k=K,l=L;}//构造函数
	bool operator<(const MyLine &A){return k<A.k;}//重定义小于号
	void print(){printf("id: %3d  k: %3d°  l: %3d\n",id,k,l);}//输出函数
};//自定义直线
//-----------------------------------------------------------------------------
int main()
{
	int i=9;//(1\2\3\4\5\6\7\8\9

	Mat src = imread(hehe[i],0);//读取图片到mat
	Mat temp,dst;//src-->temp-->dst(src为原图；temp是src经canny提取边缘的图，
	//用于下面霍夫变换；dst是最终结果图，要在temp灰度图的基础上变为彩色图才能呈现画线效果）
 
	Canny(src, temp, 10, 140, 3);//提取边缘(如果不边缘提取就会浪费巨大时间）
    cvtColor(temp, dst, CV_GRAY2BGR);//将边缘提取的灰度图转换为BGR图便于画线


	//储存检测圆的容器  
	std::vector<Vec3f> circles; 
	//调用Hough变换检测圆  
	//参数为：待检测图像，检测结果，检测方法（这个参数唯一）,累加器的分辨率，两个圆间的距离，canny门限的上限（下限自动设为上限的一半），圆心所需要的最小的投票数，最大和最小半径  
	HoughCircles(temp,circles,CV_HOUGH_GRADIENT,2,50,200,100,100,300);  
	//找出圆盘（因为最大的不一定是的，所以加了几个限制条件）
	int pos=0;
	int max=-1;
	for(size_t i = 0; i < circles.size(); i++ )
	{  
		Vec3f f=circles[i];
		if(f[2]>max && f[0]+f[2]<temp.rows && f[0]-f[2]>=0 && f[1]+f[2]<temp.cols && f[1]-f[2]>0)
		{
			max=f[2];
			pos=i;
		}
	} 
	Point center(circles[pos][0],circles[pos][1]);//找到的圆心
	int   radius= circles[pos][2];//找到的半径
	circle(dst,center,radius,Scalar(255),2);  

	list<MyLine> list_MyLine;
	vector<Vec4i> lines2;//线段检测
    HoughLinesP(temp, lines2, 1, CV_PI/180, 50, 50, 10 );
    for( size_t i = 0; i < lines2.size(); i++ )
    {
        Vec4i l = lines2[i];
		Point A(l[0], l[1]),B(l[2], l[3]);
		if(DistancetoSegment(center,A,B)<30)//根据圆心到指针的距离阈值滤掉其他线段
		{
			bool down=(A.y+B.y-2*center.y>0);//判断长的在过圆心的水平线上部还是下部
			if(A.x==B.x){//斜率为无穷的情况
				list_MyLine.push_back(MyLine(i,90+(down?180:0),Length(Point(A.x-B.x,A.y-B.y))));
			}else if(A.y==B.y){//水平的情况
				list_MyLine.push_back(MyLine(i,A.x+B.x-2*center.x>0 ? 0:180,Length(Point(A.x-B.x,A.y-B.y))));
			}else{
				if(down){
					if(A.y>center.y)
						list_MyLine.push_back(MyLine(i,360-atan2(A.y-B.y,A.x-B.x)*180/PI,Length(Point(A.x-B.x,A.y-B.y))));
					else 
						list_MyLine.push_back(MyLine(i,360-atan2(B.y-A.y,B.x-A.x)*180/PI,Length(Point(A.x-B.x,A.y-B.y))));
				}else{
					if(A.y<center.y)
						list_MyLine.push_back(MyLine(i,abs(atan2(A.y-B.y,A.x-B.x)*180/PI),Length(Point(A.x-B.x,A.y-B.y))));
					else 
						list_MyLine.push_back(MyLine(i,abs(atan2(B.y-A.y,B.x-A.x)*180/PI),Length(Point(A.x-B.x,A.y-B.y))));
				}
			}
			line(dst,A,B, Scalar(0,0,i*20+40), 2, CV_AA);
		}
			
    }

	//根据角度区分所属指针
	int now_k,pre_k=720;//当前线段的角度和前一个线段的角度
	int num=0;//指针数（可能为2或3）
	int Du[3]={0};//3个指针的度数（每组的平均）
	int Le[3]={0};//Le[i]=Le_ping[i]*0.2+le_max[i]*0.8;
	int Le_ping[3]={0};//3个指针的长度（每组的平均）
	int Le_max[3]={0};//3个指针的长度（每组区最大的）
	int t_num=0;//每组的数量（求平均用）
	MyLine now_Line;
	list_MyLine.push_back(MyLine(99,888,0));//向其中插入一个右边界这样方便处理
	list_MyLine.sort();
	while(!list_MyLine.empty())
	{
		now_Line=list_MyLine.front();
		now_k=now_Line.k;
		if(abs(now_k-pre_k)>10)//两个角度之差小于10°的算是同一类
		{
			if(num!=0){//对本组的度数和长度求平均
				Du[num-1]/=t_num;
				Le_ping[num-1]/=t_num;
				Le[num-1]=Le_ping[num-1]*0.2+Le_max[num-1]*0.8;
			}
			if(now_k==888)break;//右边界直接跳出
			t_num=0;//重新统计下一组
			num++;//组数增加1
			cout<<"---------------------------\n";//输出分割线
		}
		t_num++;//组内多一条线
		Du[num-1]+=now_Line.k;
		Le_ping[num-1]+=now_Line.l;
		if(now_Line.l>Le_max[num-1])Le_max[num-1]=now_Line.l;
		now_Line.print();
		list_MyLine.pop_front();
		pre_k=now_k;
	}
	cout<<"---------------------------\n\n";
    
	cout<<"---------------------------\n";
	int t;
	for(int i=0;i<num-1;i++){
		for(int j=i+1;j<num;j++){
			if(Le[i]>Le[j]){
				t=Le[i],Le[i]=Le[j],Le[j]=t;
				t=Du[i],Du[i]=Du[j],Du[j]=t;
			}//if end
		}//for end
	}//for end
	char s[3][10]={"hour  :","minute:","second:"};
	for(int i=0;i<num;i++)
		printf("%s  k: %3d°  l: %3d\n",s[i],Du[i],Le[i]);
	cout<<"---------------------------\n";
	if(num==2)printf("time is: %2d:%2d\n",(360-Du[0]+90)%360/30,(360-Du[1]+90)%360/6);
	else if(num==3)printf("time is: %2d:%2d:%2d\n",(360-Du[0]+90)%360/30,(360-Du[1]+90)%360/6,(360-Du[2]+90)%360/6);
	cout<<"---------------------------\n";

	imshow("src", src);
	imshow("dst", dst);
	waitKey();	
	
    return 0;
}


