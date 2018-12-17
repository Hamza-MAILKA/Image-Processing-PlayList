#include <iostream>
#include <fstream>  // file processing  
#include <stdlib.h>
#include <map>
#include <vector>
#include <math.h>
using namespace std;

typedef unsigned char   BYTE;   //  1 byte (0~255) 
typedef unsigned short  WORD;   //  2 bytes (0~65536) 
typedef unsigned long   DWORD;  //4 bytes (0~2^32 -1) 

#pragma pack(push)	//store 
#pragma pack(2)		//2-bytes aligned
struct INFO
{
	// BITMAPFILEHEADER (14 bytes) from 16 reducing to 14
	WORD bfType;          //BM -> 0x4d42 (19778)     
	DWORD BfSize;         //�`���ɤj�p        
	WORD bfReserved1;     //bfReserved1 ����0  
    WORD bfReserved2;     //bfReserved2 ����0        
    DWORD bfOffBits;      //�����q 
	// BITMAPINFOHEADER(40 bytes)    
    DWORD biSize;         //info header�j�p     
    int biWidth;
    int biHeight;
    WORD biPlanes;        //�줸�ϼh��=1 
    WORD biBitCount;      //�C��pixel�ݭn�h��bits 
    DWORD biCompression;  //0�������Y 
    DWORD biSizeImage;    //�I�}�ϸ�Ƥj�p  
    int biXPelsPerMeter;  //�����ѪR�� 
    int biYPelsPerMeter;  //�����ѪR�� 
    DWORD biClrUsed;      //0���ϥΩҦ��զ�L�C�� 
    DWORD biClrImportant; //���n���C���(0���Ҧ��C��Ҥ@�˭��n) 
};
#pragma pack(pop)  	//restore

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
			//cout<<"���ɤj�p�G "<<f.tellg()<<"bytes"<<endl;
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

/*
alpha blending
1. blend two images into one image
2. require two images, one for background (bg), the other for foregorund (fg)
3. simple linear combination of two images
4. blending equation: output=alpha*fg + (1-alpha)*bg
5. alpha means transparency, where 0<=alpha<=1
6. alpha=0, only show the bg
7. alpha=1, only show the fg
*/

Image blending(Image fg,Image bg,float alpha)
{
	Image output(fg.height,fg.width);
	for(int y=0;y<fg.height;y++)
		for(int x=0;x<fg.rowsize;x+=3)
		{
			output.term[y*fg.rowsize+x]=int(alpha*fg.term[y*fg.rowsize+x]+(1-alpha)*bg.term[y*bg.rowsize+x]);
			output.term[y*fg.rowsize+x+1]=int(alpha*fg.term[y*fg.rowsize+x+1]+(1-alpha)*bg.term[y*bg.rowsize+x+1]);
			output.term[y*fg.rowsize+x+2]=int(alpha*fg.term[y*fg.rowsize+x+2]+(1-alpha)*bg.term[y*bg.rowsize+x+2]);	
		}	
	return output;
}

int main()
{ 
	system("mode con: lines=10 cols=50");
	Image input,bg,output;
	input.load("_face.bmp");
	bg.load("white.bmp");
	char fname[15]="alpha(0.2).bmp";
	
	for(int i=0;i<=10;i+=1)
	{
		output=blending(input,bg,i*1.0/10);
		if(i==0)
			output.save("alpha(0).bmp");
		else if(i==10)
			output.save("alpha(1).bmp");
		else
		{
			fname[8]=i+48;
			output.save(fname);
		}	
	}	
	system("Pause");
	return 0;
}

