#include <iostream>
#include <fstream>  // file processing  
#include <stdlib.h>
using namespace std;

typedef unsigned char   BYTE;   //  1 byte (0~255) 
typedef unsigned short  WORD;   //  2 bytes (0~65536) 
typedef unsigned long   DWORD;  //  4 bytes (0~2^32 -1) 

// int -> 4 bytes
 
#pragma pack(push)  //�b���ܬ�2�e���x�s��Ӫ��]�w 
#pragma pack(2)    //�H 2 bytes ��� 
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
#pragma pack(pop)        //��_��Ӫ��]�w 

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

Image watermark(Image input,Image mark,int lx,int ly,int R,int G,int B) 
{
	Image output=Image(input.height,input.width);
	int height=input.height;
	int rowsize=input.rowsize;

	for(int y=0;y<height;y++)
	{
		for(int x=0;x<rowsize;x+=3)
		{
			if(y<ly || y>=(ly+mark.height) || x<lx*3 || x>=(lx+mark.width)*3)
			{
				output.term[y*rowsize+x]=input.term[y*rowsize+x];
				output.term[y*rowsize+x+1]=input.term[y*rowsize+x+1];
				output.term[y*rowsize+x+2]=input.term[y*rowsize+x+2];
			}
			
			else   
			{	//if -> remove the white background of the mark 
				if(mark.term[(y-ly)*mark.rowsize+x-3*lx]!=BYTE(0) && mark.term[(y-ly)*mark.rowsize+x+1-3*lx]!=BYTE(0) && mark.term[(y-ly)*mark.rowsize+x+2-3*lx]!=BYTE(0))
				{
					output.term[y*rowsize+x]=input.term[y*rowsize+x];
					output.term[y*rowsize+x+1]=input.term[y*rowsize+x+1];
					output.term[y*rowsize+x+2]=input.term[y*rowsize+x+2];
				}
				else
				{
					output.term[y*rowsize+x]=BYTE(B);
					output.term[y*rowsize+x+1]=BYTE(G);
					output.term[y*rowsize+x+2]=BYTE(R);
				}
			}
		}
	}	
	return output;
}

int main()
{ 
	Image input,input2,go,output;
	char image[80],mark[80],temp[80];
	int x,y,R,G,B;
	printf("Input an image : ");
	scanf("%s",image);
	for(int i=0;i<80;i++)
		temp[i]=image[i];
	input.load(image);
	int cnt=0;

	while(1)
	{
		printf("Input a mark (Q quit): ");
		scanf("%s",mark);
		if(mark[0]=='Q') 
		{
			if(cnt==0)
			{
				output.load(temp);
				output.save("output.bmp");
			}	
			break;
		}
		
		input2.load(mark);
		printf("x y R G B : ");
		scanf("%d %d %x%x%x",&x,&y,&R,&G,&B);
		output=watermark(input,input2,x,y,R,G,B);
		input=output;	
		cnt++;	
	}
	output.save("output.bmp");
			
	system("Pause");
	return 0;
}

