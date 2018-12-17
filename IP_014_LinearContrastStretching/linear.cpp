#include <iostream>
#include <fstream>  // file processing  
#include <stdlib.h>
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
			for(int y=0; y<height; y++)
				for(int x=0; x<width; x++)
					term[y*rowsize+3*x]=term[y*rowsize+3*x+1]=term[y*rowsize+3*x+2]= 255;
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

//linear scaling
//contrast stretching
Image stretching(Image input)
{
	Image output(input.height,input.width);
	
	//copy input to output
	for(int y=0;y<output.height;y++)
		for(int x=0;x<output.rowsize;x+=3)
			output.term[y*output.rowsize+x]=output.term[y*output.rowsize+x+1]=output.term[y*output.rowsize+x+2]=input.term[y*input.rowsize+x];
	
	//first, find the maximum and minimum value in the input image 
	int max=-1,min=256;
	for(int y=0;y<output.height;y++)
		for(int x=0;x<output.rowsize;x+=3)
		{
			if(output.term[y*output.rowsize+x]>max)	max=output.term[y*output.rowsize+x];
			if(output.term[y*output.rowsize+x]<min)	min=output.term[y*output.rowsize+x];
		}

	//second, apply the formula: new=(old-min)*((255-0)/(max-min))+0
	for(int y=0;y<output.height;y++)
		for(int x=0;x<output.rowsize;x+=3)
		{
			output.term[y*output.rowsize+x]=output.term[y*output.rowsize+x+1]=output.term[y*output.rowsize+x+2]=
			(output.term[y*output.rowsize+x]-min)*((255-0)*1.0/(max-min))+0;
		}
		
	return output;
}

int main()
{
	Image input,output;
	
	input.load("face.bmp");
	output=stretching(input);
	output.save("stretching.bmp");
	
	return 0;
}
