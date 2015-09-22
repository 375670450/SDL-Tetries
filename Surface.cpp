#include "Surface.h"
#define STDWIDTH  63
#define STDHEIGHT 82



using namespace std;

//-------ScreenSurface类方法-------------//

int ScreenSurface::ScreenCount = 0;

ScreenSurface::ScreenSurface():width(640),height(480),bpp(32),flags(0),WindowName(0){
	if( ScreenCount > 1 ){
		throw "Do Not Create More Than one ScreenSurface";
	}
	if( SDL_Init(SDL_INIT_VIDEO)==-1 ){
		throw SDL_GetError();
	}
	pScreen = SDL_SetVideoMode(width,height,bpp,flags);
	ScreenCount++;
}

ScreenSurface::ScreenSurface(int w,int h,int b,Uint32 f,char* window_name):
width(w),height(h),bpp(b),flags(f){
	if( ScreenCount > 1 ){
		throw "Do Not Create More Than one ScreenSurface";
	}
	if( SDL_Init(SDL_INIT_VIDEO)==-1 ){
		throw SDL_GetError();
	}
	pScreen = SDL_SetVideoMode(width,height,bpp,flags);
	ScreenCount++;
	if( window_name!=0 ){
        WindowName = window_name;
        SDL_WM_SetCaption(WindowName, 0);
	}
}

ScreenSurface::~ScreenSurface(){
	ScreenCount--;
	SDL_Quit();
}

SDL_Surface * ScreenSurface::point()const{
	return pScreen;
}

bool ScreenSurface::flip() const{
	if( SDL_Flip(pScreen)==-1 ){
		return false;
	}else
		return true;
}



void ScreenSurface::fillColor(Uint8 r, Uint8 g, Uint8 b) const
{
     if ( SDL_FillRect(pScreen, 0, SDL_MapRGB(pScreen->format, r, g, b)) < 0 )
         throw ErrorInfo(SDL_GetError());
}

void ScreenSurface::bar(int left,int top,int width,int height,Uint32 color, Uint8 a) const{
    Uint8 r,g,b;
    SDL_Rect tmpRect;
    tmpRect.x = left;
    tmpRect.y = top;
    tmpRect.w = width;
    tmpRect.h = height;
    r = (color>>2*8)&0xff;
    g = (color>>8)&0xff;
    b = (color)&0xff;
    if ( SDL_FillRect(pScreen, &tmpRect, SDL_MapRGBA(pScreen->format, r, g, b, a)) < 0 )
         throw ErrorInfo(SDL_GetError());
}

void ScreenSurface::flash(int left,int top,int width,int height) const{
    Uint8 r,g,b;
    SDL_Rect tmpRect;
    tmpRect.x = left;
    tmpRect.y = top;
    tmpRect.w = width;
    tmpRect.h = height;
    r = g = b = 0xff;
    while( r-- ){
        g = b = r;
        SDL_FillRect(pScreen, &tmpRect, SDL_MapRGB(pScreen->format, r, g, b));
        flip();
        SDL_Delay(1);
    }
}
void ScreenSurface::flash(int x,int y,int xy[4][2]) const{
    int left,top,k;
    Uint8 r,g,b;
    SDL_Rect tmpRect;
    tmpRect.w = CS-2;
    tmpRect.h = CS-2;
    r = g = b = 0xff;
    while( r-- ){
        g = b = r;
        for(k=0; k<4; k++){
            left = BX+(x + xy[k][0])*CS;
            top  = BY+(y + xy[k][1])*CS;
            tmpRect.x = left+1;
            tmpRect.y = top+1;
            SDL_FillRect(pScreen, &tmpRect, SDL_MapRGB(pScreen->format, r, g, b));
        }
        flip();
        SDL_Delay(1);
    }
}

Uint32 ScreenSurface::getpixel(int x,int y) const{
    int bpp = pScreen->format->BytesPerPixel;
    Uint8 *p = (Uint8*)(pScreen->pixels) + y*(pScreen->pitch) + x*bpp;
    switch( bpp ){
        case 1: return *p;
        case 2: return *(Uint16 *)p;
        case 3: if(SDL_BYTEORDER==SDL_BIG_ENDIAN)           //大端模式，高位低地址，低位高地址(与书写方向相同),p[2]地址高于p[0]
                    return p[0]<<16 | p[1] << 8 | p[2];
                else                                        //小端模式，高位高地址，低位低地址(与书写方向相反)
                    return p[2]<<16 | p[1] << 8 | p[0];
        case 4: return *(Uint32 *)p;
        default:return 0xffffffff;           //shouldn't happen
    }
}

void ScreenSurface::line(int x1,int y1,int x2,int y2,Uint32 color) const{
    Draw_Line(pScreen,x1,y1,x2,y2,color);
}

void ScreenSurface::rectangle(int x1,int y1,int x2,int y2,Uint32 color) const{
    Draw_Rect(pScreen,x1,y1,x2-x1,y2-y1,color);
}

//---------DisplaySurface类方法---------------//



int DisplaySurface::textNum = 0;

DisplaySurface::DisplaySurface(const ScreenSurface& screen, const std::string& file_name ):
fileName(file_name), pFont(0)
{
    SDL_Surface* pSurfaceTemp = IMG_Load(file_name.c_str());
    if ( pSurfaceTemp == 0 )
        throw ErrorInfo(SDL_GetError());
    pSurface = SDL_DisplayFormatAlpha(pSurfaceTemp);
    if ( pSurface == 0 )
        throw ErrorInfo(SDL_GetError());
    SDL_FreeSurface(pSurfaceTemp);
    pScreen = screen.point();
}

DisplaySurface::~DisplaySurface()
{
    SDL_FreeSurface(pSurface);
}

SDL_Surface* DisplaySurface::point() const
{
    return pSurface;
}

void DisplaySurface::blit() const
{
    if ( SDL_BlitSurface(pSurface, 0, pScreen, 0) < 0 )
        throw ErrorInfo(SDL_GetError());
}


void DisplaySurface::blit(int at_x, int at_y) const
{
    SDL_Rect offset;
    offset.x = at_x;
    offset.y = at_y;
    if ( SDL_BlitSurface(pSurface, 0, pScreen, &offset) < 0 )
        throw ErrorInfo(SDL_GetError());
}

void DisplaySurface::blit(int at_x, int at_y,
                          int from_x, int from_y, int w, int h,
                          int delta_x, int delta_y) const
{
    SDL_Rect offset;
    offset.x = at_x - delta_x;
    offset.y = at_y - delta_y;

    SDL_Rect dest;
    dest.x = from_x - delta_x;
    dest.y = from_y - delta_y;
    dest.w = w + delta_x*2;
    dest.h = h + delta_y*2;

    if ( SDL_BlitSurface(pSurface, &dest, pScreen, &offset) < 0 )
        throw ErrorInfo(SDL_GetError());
}

bool DisplaySurface::ChangeColor(int flag){
    string filename;
    if( flag==0 ) filename = "Common.png";
    else filename = "Selected.png";
    SDL_Surface* pSurfaceTemp = IMG_Load(filename.c_str());
    if ( pSurfaceTemp == 0 )
        return false;
//        throw SDL_GetError();
    pSurface = SDL_DisplayFormatAlpha(pSurfaceTemp);
    if ( pSurface == 0 )
        return false;
//        throw SDL_GetError();
    SDL_FreeSurface(pSurfaceTemp);
    return true;
}

void DisplaySurface::colorKey(Uint8 r, Uint8 g, Uint8 b, Uint32 flag)
{
    Uint32 colorkey = SDL_MapRGB(pSurface->format, r, g, b);
    if ( SDL_SetColorKey(pSurface, flag, colorkey ) < 0 )
        throw ErrorInfo(SDL_GetError());
}

//---------------TextSurface类方法--------------------//


//for TextSurface
DisplaySurface::DisplaySurface(const std::string& msg_name, const std::string& message, const ScreenSurface& screen,
                    Uint8 r, Uint8 g , Uint8 b,
                    const std::string& ttf_fileName, int ttf_size):
fileName(msg_name)
{
    if ( textNum == 0 )
        if ( TTF_Init() < 0 )
            throw ErrorInfo("TTF_Init() failed!");

    SDL_Color textColor;
    textColor.r = r;
    textColor.g = g;
    textColor.b = b;

    pFont = TTF_OpenFont(ttf_fileName.c_str(), ttf_size);
    if ( pFont == 0 )
        throw ErrorInfo("TTF_OpenFont() failed!");

    pSurface = TTF_RenderText_Solid(pFont, message.c_str(), textColor);
    if ( pSurface == 0 )
        throw ErrorInfo("TTF_RenderText_solid() failed!");
    pScreen = screen.point();

    textNum++;
}

int DisplaySurface::tellTextNum() const
{
    return textNum;
}

void DisplaySurface::reduceTextNum()
{
    textNum--;
}

void DisplaySurface::deleteFontPoint()
{
    TTF_CloseFont(pFont);
}


TextSurface::TextSurface(const std::string& msg_name, const std::string& message, const ScreenSurface& screen,
                    Uint8 r, Uint8 g, Uint8 b,
                    const std::string& ttf_fileName, int ttf_size):
DisplaySurface(msg_name, message, screen, r, g, b, ttf_fileName, ttf_size)
{}

TextSurface::~TextSurface()
{
    deleteFontPoint();
    reduceTextNum();
    if ( tellTextNum() == 0 )
        TTF_Quit();
}



//----------ErrorInfo类方法--------------------------//






