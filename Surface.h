#ifndef SURFACE_H
#define SURFACE_H

#include <iostream>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_draw.h"

#define BOX_R 20 /*��������*/
#define BOX_C 10 /*��������*/

#define CS 20 /*����Ĵ�С*/
#define BX 100 /*�������Ͻǵ�x����*/
#define BY 100 /*�������Ͻǵ�y����*/
#define BSR 20*20 /*�����Ĵ�С*/
#define BSC 10*20 /*�����Ĵ�С*/
#define GAP 600   /*���������ļ��*/
#define CURBLKNUM 28 /*��ǰʹ�õķ������*/


using namespace std;

//��Ļ�ӿ�Surface
class ScreenSurface{
	private:
		static int ScreenCount;	//��ǰ������
		int width;				//���ڿ��
		int height;				//���ڸ߶�
		int bpp;				//λ���
		Uint32 flags;
		SDL_Surface *pScreen;
		char *WindowName;
	public:
		ScreenSurface();
		ScreenSurface(int w,int h,int b = 32,Uint32 f = 0,char* window_name = 0);
		~ScreenSurface();
		SDL_Surface * point() const;	//�÷������ض����е�pScreen
		void fillColor(Uint8 r, Uint8 g, Uint8 b) const;
		void bar(int left,int top,int width,int height,Uint32 = 0, Uint8 a = 0) const;
		void line(int x1,int y1,int x2,int y2,Uint32 color = 0) const;
		void rectangle(int x1,int y1,int x2,int y2,Uint32 color = 0) const;
		void flash(int left,int top,int width,int height) const;
		void flash(int x,int y,int xy[4][2]) const;
		Uint32 getpixel(int x,int y) const;
		bool flip() const;				//�÷������ڰ�screen surface��ʾ����
};

// ͼƬ��ʾSurface
class DisplaySurface
{
private:
    std::string fileName;
    SDL_Surface* pSurface;
    SDL_Surface* pScreen;
    //for TextSurafce
    static int textNum;
    TTF_Font* pFont;
public:
    DisplaySurface(const ScreenSurface& screen, const std::string& file_name = "Common.png");
    ~DisplaySurface();
    SDL_Surface* point() const;
    void blit() const;
    void blit(int at_x, int at_y) const;
    void blit(int at_x, int at_y, int from_x, int from_y, int w, int h, int delta_x = 0, int delta_y = 0) const;
    void colorKey(Uint8 r = 0, Uint8 g = 0xFF, Uint8 b = 0xFF, Uint32 flag = SDL_SRCCOLORKEY);
    bool ChangeColor(int flag = 0);			//block transfer
    bool ButtonDown();
protected:
    //for TextSurface
    DisplaySurface(const std::string& msg_name, const std::string& message, const ScreenSurface& screen,
                    Uint8 r=0xff, Uint8 g=0xff, Uint8 b=0xff,
                    const std::string& ttf_fileName="arial.ttf", int ttf_size=20);
    int tellTextNum() const;
    void reduceTextNum();
    void deleteFontPoint();
};

class TextSurface: public DisplaySurface
{
public:
    TextSurface(const std::string& msg_name, const std::string& message, const ScreenSurface& screen,
                    Uint8 r = 0, Uint8 g = 0, Uint8 b = 0,
                    const std::string& ttf_fileName = "arial.ttf", int ttf_size = 15);
    ~TextSurface();
};

class ErrorInfo
{
private:
    std::string info;
public:
    ErrorInfo():info("Unknown ERROR!")
    {}
    ErrorInfo(const char* c_str)
    {
        info = std::string(c_str);
    }
    ErrorInfo(const std::string& str):info(str)
    {}
    void show() const
    {
        std::cerr << info << std::endl;
    }
};

class Block{
public:
        int x;
        int y;
        int xy[4][2];                       //ÿһ��ĸ�������(��Ϊÿ�ַ���̶���4�����ӹ���)
        int type;                           //����������ʶ��(һ����BLKNUM/10��) == number/4
        static int globalTemp;
public:
        int number;                         //����ı�ʶ��(0~BLKNUM)
        int color;
        Block();
        int Reset(int newx=4,int newy=0);   //���÷��������λ��(x,y)
        int isOk() const;                   //�жϸ�λ���Ƿ���ƶ�
        int isOk(int nextX,int next) const;//�жϸ�λ���Ƿ���ƶ�
        int DrawCube() const;               //����������(flag=0���ʾ����ͨ���飬����ΪԤ���ķ���)
        int ClearCube() const;              //����������
        int MoveDown();                     //�����ƶ�
        int MoveLeft();                     //�����ƶ�
        int MoveRight();                    //�����ƶ�
        int Rotate();                       //��ת
        int TurnNext();

};

class AIBlock:public Block{
public:
    static int AIglobalTemp;
    AIBlock();
    int Reset(int newx=4,int newy=0);   //���÷��������λ��(x,y)
    int isOk() const;                   //�жϸ�λ���Ƿ���ƶ�
    int isOk(int nextX,int next) const; //�жϸ�λ���Ƿ���ƶ�
    int DrawCube() const;               //����������(flag=0���ʾ����ͨ���飬����ΪԤ���ķ���)
    int ClearCube() const;              //����������
    int MoveDown();                     //�����ƶ�
    int MoveLeft();                     //�����ƶ�
    int MoveRight();                    //�����ƶ�
    int Rotate();                       //��ת
    int  Evaluate(int curBox[BOX_C][BOX_R],int Path[BOX_C][BOX_R]);
    void DFS(Block blk,int Path[BOX_C][BOX_R],int Evaluated[BOX_C][BOX_R]);
    void Search();
 };


#endif // SURFACE_H
