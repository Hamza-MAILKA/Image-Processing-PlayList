#include <iostream>
#include <fstream> 
#include <cmath>
#include <conio.h>
using namespace std;

typedef unsigned char   BYTE;   //  1 byte (0~255) 
typedef unsigned short  WORD;   //  2 bytes (0~65536) 
typedef unsigned long   DWORD;  //  4 bytes (0~2^32 -1) 

#pragma pack(push) 
#pragma pack(2)    
struct INFO
{
	// BITMAPFILEHEADER (14 bytes) from 16 reducing to 14
	WORD bfType;          //BM -> 0x4d42 (19778)     
	DWORD BfSize;         //總圖檔大小        
	WORD bfReserved1;     //bfReserved1 須為0  
    WORD bfReserved2;     //bfReserved2 須為0        
    DWORD bfOffBits;      //偏移量 
	// BITMAPINFOHEADER(40 bytes)    
    DWORD biSize;         //info header大小     
    int biWidth;
    int biHeight;
    WORD biPlanes;        //位元圖層數=1 
    WORD biBitCount;      //每個pixel需要多少bits 
    DWORD biCompression;  //0為不壓縮 
    DWORD biSizeImage;    //點陣圖資料大小  
    int biXPelsPerMeter;  //水平解析度 
    int biYPelsPerMeter;  //垂直解析度 
    DWORD biClrUsed;      //0為使用所有調色盤顏色 
    DWORD biClrImportant; //重要的顏色數(0為所有顏色皆一樣重要) 
};
#pragma pack(pop) 

class Image
{	
	public:
		
		int height;
		int width;
		int rowsize;    // bgr -> 3 bytes(24 bits) 
		BYTE* term;
		
		Image()   //storage is bottom-up,from left to right 
		{
			height=0;
			width=0;
		}
		
		Image(int height,int width)
		{
			this->height=height;
			this->width=width;
			rowsize=(3*width+3)/4*4;   //set to be a multiple of "4" 
			term=new BYTE[height*rowsize];  
		}
		
		void load(const char *filename)
		{
			INFO h;  
			ifstream f;
			f.open(filename,ios::in|ios::binary);
			f.seekg(0,f.end);
			//cout<<"圖檔大小： "<<f.tellg()<<"bytes"<<endl;
			f.seekg(0,f.beg);
			f.read((char*)&h,sizeof(h));
						
			width=h.biWidth;
			height=h.biHeight;
			//cout<<"reading from "<<filename<<"..."<<endl;
			//print(h);
			*this=Image(height,width);
			f.read((char*)term,height*rowsize);
			f.close();
		}
		
		void save(const char* filename)
		{
			INFO h=
			{		
				19778,   //0x4d42
				DWORD(54+rowsize*height),   
				0,
				0,
				54,
				40,
				width,
				height,
				1,
				24,   
				0,
				DWORD(rowsize*height),
				3780,   //3780
				3780,   //3780
				0,
				0				
			};
			//cout<<"writing into "<<filename<<"..."<<endl;
			ofstream f;
			f.open(filename,ios::out|ios::binary);
			f.write((char*)&h,sizeof(h));
			f.write((char*)term,rowsize*height);
			f.close();	
		}	
};

void setcolor(Image m,int x,int y,int bow)  // 0 -> white ; 1 -> black
{
	if(bow==0)
		m.term[y*m.rowsize+x]=m.term[y*m.rowsize+x+1]=m.term[y*m.rowsize+x+2]=BYTE(255);
	else
		m.term[y*m.rowsize+x]=m.term[y*m.rowsize+x+1]=m.term[y*m.rowsize+x+2]=BYTE(0);
}

int size=400;

Image NNI(Image input,int w,int h)	//nearest neighbor interpolation
{
	Image output(h,w);
	int nx,ny;
	for(int y=0;y<output.height;y++)
		for(int x=0;x<output.rowsize;x+=3)	
		{
			//set to be a multiple of "3" 
			nx=(int((int(x*input.width*1.0/output.width)+2)/3*3));
			ny=int(y*input.height*1.0/output.height);
			output.term[y*output.rowsize+x]=input.term[ny*input.rowsize+nx];
			output.term[y*output.rowsize+x+1]=input.term[ny*input.rowsize+nx+1];
			output.term[y*output.rowsize+x+2]=input.term[ny*input.rowsize+nx+2];
		}
	return output;
}

Image BI(Image input,int w,int h)	//bilinear interpolation
{
	Image output(h,w);
	int nx,ny,x1,x2,x3,x4,y1,y2,y3,y4,c1,c2,c3,c4;
	float dx,dy;
	for(int y=0;y<output.height;y++)
		for(int x=0;x<output.rowsize;x+=3)
		{
			x1=(int((int(x*input.width*1.0/output.width)+2)/3*3));
			y1=int(y*input.height*1.0/output.height);
			x2=x1+3;	y2=y1;
			x3=x1;		y3=y1+1;
			x4=x1+3;	y4=y1+1;
			if(x2>=input.rowsize)	x2-=3;
			if(y3>=input.height)	y3--;
			if(x4>=input.rowsize)	x4-=3;
			if(y4>=input.height)	y4--;
			dx=1.0/abs(x1-x)/3;
			dy=1.0/abs(y1-y);
			for(int i=0;i<3;i++)
			{
				c1=input.term[y1*input.rowsize+x1+i];
				c2=input.term[y2*input.rowsize+x2+i];
				c3=input.term[y3*input.rowsize+x3+i];
				c4=input.term[y4*input.rowsize+x4+i];
				output.term[y*output.rowsize+x+i]=int((c1*(1-dx)*(1-dy)+c2*(dx)*(1-dy)+c3*(1-dx)*(dy)+c4*(dx)*(dy)));
			}
		}
	return output;
}

Image circle(Image input,int x,int y)
{
	Image output=input;
	double theta;
	for(int i=0;i<360;i++)
	{
		theta=M_PI*1.0/360.0*i;
		//(x+rcos(theta), y+rsin(theta)) 
		setcolor(output,int((x+200*cos(theta))*3),int(y+200*sin(theta)),0);
		setcolor(output,int((x+200*cos(theta))*3),int(y-200*sin(theta)),0);
	}
	return output;
} 

Image cut(Image input,Image mask,int dx,int dy)
{
	Image output(size,size);
	double theta;
	int flag;
	//cutting
	for(int y=0,y2=dy-size/2;y<output.height,y2<dy+size/2;y++,y2++)
		for(int x=0,x2=(dx-size/2)*3;x<output.rowsize,x2<(dx+size/2)*3;x+=3,x2+=3)
		{
			output.term[y*output.rowsize+x]=input.term[y2*input.rowsize+x2];
			output.term[y*output.rowsize+x+1]=input.term[y2*input.rowsize+x2+1];	
			output.term[y*output.rowsize+x+2]=input.term[y2*input.rowsize+x2+2];
		}
	
	for(int y=0;y<output.height;y++)
		for(int x=0;x<output.rowsize;x+=3)
		{
			//maintain the color of black region
			if(mask.term[y*mask.rowsize+x]!=0 && mask.term[y*mask.rowsize+x+1]!=0 && mask.term[y*mask.rowsize+x+2]!=0)
			{		
				output.term[y*output.rowsize+x]=mask.term[y*mask.rowsize+x];
				output.term[y*output.rowsize+x+1]=mask.term[y*mask.rowsize+x+1];	
				output.term[y*output.rowsize+x+2]=mask.term[y*mask.rowsize+x+2];
			}
		}
	return output;
}

int main()
{ 
	system("mode con: lines=20 cols=60");
	Image input,output,final,mask;
	char filename[80],arrow,mname[80];
	int qt=0,width,height,flag;
	cout<<"Simulation of uploading the profile picture in Instagram..."<<endl;
	cout<<"arrow key for moving, p(capture), q(quit)"<<endl; 	
	
	//select an image
	cout<<"Input an image: ";
	cin>>filename;
	cout<<"Input width & height of the resized image: ";
	scanf("%d%d",&width,&height);
	cout<<"1(NNI), 0(BI): ";
	cin>>flag;
	if(width<size)	width=size;
	if(height<size)	height=size;
	
	while(1)
	{
		input.load(filename);
		Image mid(height,width);
		//resized image
		if(flag)	mid=NNI(input,width,height);
		else mid=BI(input,width,height);
		mid.save("resized.bmp");
		
		static int dx=size/2;
		static int dy=mid.height-size/2;
		
		arrow=getch();
		switch(arrow)
		{
			case 72:	//up
				{
					dy+=10;
					if(dy+(size/2)>=mid.height)	dy-=10;
					cout<<"^";
					break;
				}
			case 80:	//down
				{
					dy-=10;
					if(dy-(size/2)<=0)	dy+=10;
					cout<<"v";
					break;
				}
			case 75:	//left
				{
					dx-=10;
					if((dx-(size/2))*3<=0)	dx+=10;
					cout<<"<";
					break;
				}
			case 77:	//right
				{
					dx+=10;
					if((dx+(size/2))*3>=mid.rowsize)	dx-=10;
					cout<<">";
					break;
				}
			case 'p':	//capture
				{
					qt=1;
					cout<<"p"<<endl;
					cout<<"Input a mask image: ";
					cin>>mname;
					mask.load(mname);
					break;
				}
			case 'q':	//quit
				{
					qt=-1;
					cout<<"q";
					break;
				}
			
		}
		if(qt==1)
		{
			final=cut(mid,mask,dx,dy);
			final.save("capture.bmp");
			qt=0;
		}
		
		else if(qt==0)
		{
			output=circle(mid,dx,dy);
			output.save("mid.bmp");
		}	
		else if(qt==-1)
			break;	
	}
	return 0;
}

