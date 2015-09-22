#ifndef SURFACE_H
#define SURFACE_H

#include <iostream>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_draw.h"

#define BOX_R 20 /*容器行数*/
#define BOX_C 10 /*容器列数*/

#define CS 20 /*方块的大小*/
#define BX 100 /*容器左上角的x坐标*/
#define BY 100 /*容器左上角的y坐标*/
#define BSR 20*20 /*容器的大小*/
#define BSC 10*20 /*容器的大小*/
#define GAP 600   /*两个容器的间距*/
#define CURBLKNUM 28 /*当前使用的方块个数*/


using namespace std;

//屏幕接口Surface
class ScreenSurface{
	private:
		static int ScreenCount;	//当前窗口数
		int width;				//窗口宽度
		int height;				//窗口高度
		int bpp;				//位深度
		Uint32 flags;
		SDL_Surface *pScreen;
		char *WindowName;
	public:
		ScreenSurface();
		ScreenSurface(int w,int h,int b = 32,Uint32 f = 0,char* window_name = 0);
		~ScreenSurface();
		SDL_Surface * point() const;	//该方法返回对象中的pScreen
		void fillColor(Uint8 r, Uint8 g, Uint8 b) const;
		void bar(int left,int top,int width,int height,Uint32 = 0, Uint8 a = 0) const;
		void line(int x1,int y1,int x2,int y2,Uint32 color = 0) const;
		void rectangle(int x1,int y1,int x2,int y2,Uint32 color = 0) const;
		void flash(int left,int top,int width,int height) const;
		void flash(int x,int y,int xy[4][2]) const;
		Uint32 getpixel(int x,int y) const;
		bool flip() const;				//该方法用于把screen surface显示出来
};

// 图片显示Surface
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
        int xy[4][2];                       //每一块的格子坐标(因为每种方块固定由4个格子构成)
        int type;                           //方块的种类标识号(一共有BLKNUM/10种) == number/4
        static int globalTemp;
public:
        int number;                         //方块的标识号(0~BLKNUM)
        int color;
        Block();
        int Reset(int newx=4,int newy=0);   //重置方块的坐标位置(x,y)
        int isOk() const;                   //判断该位置是否可移动
        int isOk(int nextX,int next) const;//判断该位置是否可移动
        int DrawCube() const;               //画出本方块(flag=0则表示是普通方块，否则为预览的方块)
        int ClearCube() const;              //擦除本方块
        int MoveDown();                     //向下移动
        int MoveLeft();                     //向左移动
        int MoveRight();                    //向右移动
        int Rotate();                       //旋转
        int TurnNext();

};

class AIBlock:public Block{
public:
    static int AIglobalTemp;
    AIBlock();
    int Reset(int newx=4,int newy=0);   //重置方块的坐标位置(x,y)
    int isOk() const;                   //判断该位置是否可移动
    int isOk(int nextX,int next) const; //判断该位置是否可移动
    int DrawCube() const;               //画出本方块(flag=0则表示是普通方块，否则为预览的方块)
    int ClearCube() const;              //擦除本方块
    int MoveDown();                     //向下移动
    int MoveLeft();                     //向左移动
    int MoveRight();                    //向右移动
    int Rotate();                       //旋转
    int  Evaluate(int curBox[BOX_C][BOX_R],int Path[BOX_C][BOX_R]);
    void DFS(Block blk,int Path[BOX_C][BOX_R],int Evaluated[BOX_C][BOX_R]);
    void Search();
 };


#endif // SURFACE_H
