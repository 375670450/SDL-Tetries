/*
    SDL 1.2.15 Tetries(using MulThread,Transplant from EGE project)
                    By Stardust
                    2015.06
    关键: Evaluate函数的返回值
    问题：1.创建TextSurface类的text对象语句崩溃：创建text对象的时候自动执行了构造函数，读取外部字体文件失败
          2.关于颜色：Uint32 color = 0x12345678: 12-α 34-r 56-g 78-b 其中每个数字是一个二位十六进制数,即一个8位的二进制数
                      因此要活的红色应该用 r = (color>>【2】*8)&0xff ;（一个颜色占8位）
          3.枚举定义中元素之间要用逗号隔开
          4.绘图的操作最好统一只放在一处(用一个队列存储要进行的操作，且该部分应该放在非主线程中才不会延迟？）
          5.游戏结束(gameOver=1)之前不要结束(KillThread)工作线程
          6.const声明的成员函数不能修改本对象的数据成员的值，因此一般是用作输出文字、图像
          7.GetPixel()无法正确读到像素：bpp = pScreen->format->BytesPerPixel 写成 BitsPerPixel
          8.无法长按:解决方法一：因为只有当检测到pollEvent=1才进入键位的判断，无法实现长按，因此要用GetKeyState
                     解决方法二：用EnableKeyRepeat(int delay,int interval)来开启按键长按的功能【效果更好】
                                 delay是按下一个键后到开始检测重复所停留的时间(400ms)，interval是两次读取按下信息的间隔(40ms)
          9.平滑处理？
          10.AI的实现：以一种新的数据结构路点为核心，先用A*算法搜索出最短路径，将路径转换为若干路点（大多数情况只需一个？），
                       之后根据这些路点进行移动即可
            路点：元素包含：目的点坐标，先横移还是纵移，目的块的形状
                   默认都是先横移后纵移，移动前需调整形状（若希望移动后才调整形状则需两个路点，
                   第一个只移动不变形，第二个才变形，目标地点相同）
            利用A*算法找出最佳位置
            思路：从最下面开始遍历(枚举位置与可变化形态），并用当前方块的type的四个方块适配
          11.把全局变量封装进Game对象
          12.找到path后无法添加进Operation: 改变y 与 path[x][y+1]的顺序错了
          13.DFS无法到x==0的位置?
          14.AIMode下还需要用path的无法选择来判断落地并令alive=0,且重新ShowCube之后记得加 if( AIMode ) Search();
          15.AIMode下有时会突然到ShowCube的时候卡住: Search所用的时间过长(其实是在循环)

          16.【重点】竖条无法移动到角落填坑: 因为Search函数中做的优化使得DFS之前就使它下落得太多? 估值函数问题?
          17.用模板类实现AIPlayer的Search,DFS?
          18.DFS不需要锁线程

*/

#include "stdlib.h"
#include "time.h"
#include "Surface.h"
#include "Blocks.h"             /*方块形态数组*/
#include "Queue.h"
#include "windows.h"
#include "Controller.h"

#define random(x) (rand()%x)
#define KEYBREAK 40

const int SCREEN_WIDTH = 1240;
const int SCREEN_HEIGHT = 600;


enum COLORS {
	BLACK           = 0,
	BLUE            = 0x0000A8,
	GREEN           = 0x00A800,
	CYAN            = 0x00A8A8,
	RED             = 0xA80000,
	MAGENTA         = 0xA800A8,
	BROWN           = 0xA8A800,
	LIGHTGRAY       = 0xA8A8A8,
	DARKGRAY        = 0x545454,
	LIGHTBLUE       = 0x5454FC,
	LIGHTGREEN      = 0x54FC54,
	LIGHTCYAN       = 0x54FCFC,
	LIGHTRED        = 0xFC5454,
	LIGHTMAGENTA    = 0xFC54FC,
	YELLOW          = 0xFCFC54,
	WHITE           = 0xFFFFFF,
};

enum OPERATIONS {
    MOVEDOWN  = 1,
    MOVELEFT  = 2,
    MOVERIGHT = 3,
    ROTATE    = 4,
    TURNNEXT  = 5,
};



class Game{
public:
    int Player;                           //标示该游戏数据对象是玩家还是AI
    int box[BOX_C][BOX_R];                //容积每个方格状态
    //    Block blk[BLKNUM];                  //备选方块数组【并不是用来操作的，用公用的就好】
//    AIBlock blk[BLKNUM];
    AIBlock curblk;                         /*当前操作的方块(省去传递指针)*/
    int next,nextColor; /*当前的方块类型代号(1~24),只有生成和旋转时改变(该全局变量就避免了给方块数据结构增加一项代号的麻烦*/
    int alive = 0;
//    int CurBlkNum = CURBLKNUM;          /*当前使用的方块个数*/
    int Search_start, Search_end;
    int BestPath[BOX_C][BOX_R];
    int BestRotate;
    int BestScore;
    int command;
    bool gameOver = false;
    Queue Operation;                      //操作执行队列(以防同时执行两个操作)
    CRITICAL_SECTION cs;
    SDL_Thread * AIMultThread;
public:
    int AIInitBox();
    int AIShowCube();
    int AIGameInitial();
    int AIDelLine();
    void Search();
    void DFS(AIBlock blk,int Path[BOX_C][BOX_R],int Evaluated[BOX_C][BOX_R]);
    friend int AIMain(void * unused);
    friend int AIThreadFunc(void * unused);             //多线程函数必须是全局函数
};



Game AIPlayer;
extern int b[BLKNUM][4][4];             //方块形态数组,详细在下面
int box[BOX_C][BOX_R];                  //容积每个方格状态
Queue Operation;                      //操作执行队列(以防同时执行两个操作)
Block blk[BLKNUM];                      //备选方块数组
Block curblk;           /*当前操作的方块(省去传递指针)*/

int Block::globalTemp = 0;
int AIBlock::AIglobalTemp = 0;
int command;
int next,nextColor; /*当前的方块类型代号(1~24),只有生成和旋转时改变(该全局变量就避免了给方块数据结构增加一项代号的麻烦*/
int count = 0;/*计数器，记录已经消除的行数*/
char score[100];
int alive = 0;
int CurBlkNum = CURBLKNUM;          /*当前使用的方块个数*/

int AIMode = 0;
int ChangeTimes;        /*可以变换的次数*/
bool gameOver = false;
int Search_start, Search_end;

CRITICAL_SECTION cs;
ScreenSurface Screen(SCREEN_WIDTH,SCREEN_HEIGHT);
SDL_Thread *MultThread = NULL;
SDL_Thread *AIThread = NULL;


void xyprintf(int x,int y,const char * format, ...);
void printBox(int Box[BOX_C][BOX_R]);
void pressESCtoQuit();
int ShowBox();
int InitBox();
int ShowScore();
int ShowCube();
int GameInitial();
int DelLine();
int ThreadFunc(void * unused);
int BestPath[BOX_C][BOX_R];
int BestRotate;
int BestScore;


int  SelfMadeBlocks(int n);
int  Evaluate(int curBox[BOX_C][BOX_R],int Path[BOX_C][BOX_R]);
void Search();
void DFS(Block blk,int Path[BOX_C][BOX_R],int Evaluated[BOX_C][BOX_R]);


int AIThreadFunc(void * unused){
    int old_time,cur_time,x,y;
    old_time = cur_time = SDL_GetTicks();
    while( !AIPlayer.gameOver ){
        while( AIPlayer.alive ){
            while( !AIPlayer.Operation.isEmpty() && AIPlayer.alive){
                EnterCriticalSection(&AIPlayer.cs);
                AIPlayer.Operation.DeleteQ(AIPlayer.command);
                AIPlayer.curblk.ClearCube();
                switch( AIPlayer.command ){
                    case MOVEDOWN: AIPlayer.curblk.MoveDown();
                                   break;
                    case MOVELEFT: AIPlayer.curblk.MoveLeft();
                                   break;
                    case MOVERIGHT:AIPlayer.curblk.MoveRight();
                                   break;
                    case ROTATE  : AIPlayer.curblk.Rotate();
                                   break;
                 }
                 if( AIPlayer.command == TURNNEXT ){                 //有的特殊命令最好特殊处理
                    AIPlayer.curblk.TurnNext();
                    break;
                 }
                 AIPlayer.curblk.DrawCube();
                 Screen.flip();
                 LeaveCriticalSection(&AIPlayer.cs);
            }
            cur_time = SDL_GetTicks();
            SDL_Delay(1);

            if( cur_time - old_time < 400 ) continue;
            old_time = cur_time;
            AIPlayer.Operation.AddQ(MOVEDOWN);
//            old_time = cur_time;
//            x = AIPlayer.curblk.x;
//            y = AIPlayer.curblk.y;
//            if( AIPlayer.BestRotate>0 ){
//                AIPlayer.Operation.AddQ(ROTATE);
//                AIPlayer.BestRotate -= 1;
//                continue;
//            }
//            if( AIPlayer.BestPath[x][y+1] ){
//                AIPlayer.Operation.AddQ(MOVEDOWN);
//                AIPlayer.BestPath[x][y+1] = 0;
//                y += 1;
//                continue;
//            }else if( BestPath[x+1][y] ){
//                AIPlayer.Operation.AddQ(MOVERIGHT);
//                AIPlayer.BestPath[x+1][y] = 0;
//                x += 1;
//                continue;
//            }else if( AIPlayer.BestPath[x-1][y] ){
//                AIPlayer.Operation.AddQ(MOVELEFT);
//                AIPlayer.BestPath[x-1][y] = 0;
//                x -= 1;
//                continue;
//            }
        }
        while(AIPlayer.AIDelLine());
        AIPlayer.Operation.ClearQ();
        AIPlayer.AIShowCube();
//        AIPlayer.Search();
        AIPlayer.alive = 1;
    }
    return 0;
}

int AIMain(void * unused){
    AIPlayer.AIGameInitial();
    AIPlayer.alive = 0;
    AIPlayer.gameOver = false;
    AIPlayer.Search_start = 0, AIPlayer.Search_end=0;

    AIPlayer.AIMultThread = SDL_CreateThread(AIThreadFunc,NULL);
    while( AIPlayer.gameOver == false ){
        SDL_Delay(100);                 //减轻CPU负担
    }
    SDL_KillThread(AIPlayer.AIMultThread);
    return 0;
}



//-------------------------------------↓【MAIN】↓-------------------------------------------//

int main(int argc ,char* argv[]){
//    freopen( "CON", "wt", stdout );
//    SelfMadeBlocks(0);
    GameInitial();

    Sint32 key;
    SDL_Event gameEvent;

    MultThread = SDL_CreateThread(ThreadFunc,NULL);
    AIThread   = SDL_CreateThread(AIMain,NULL);
    while( gameOver == false ){
        //press ESC or click X to quit.
        while( SDL_PollEvent(&gameEvent) != 0 ){
            if ( gameEvent.type == SDL_QUIT ){
                gameOver = true;
            }
            if ( gameEvent.type == SDL_KEYDOWN ){
                if ( gameEvent.key.keysym.sym == SDLK_ESCAPE ){
                    gameOver = true;
                    return 0;
                }
                key = gameEvent.key.keysym.sym;
                if( key==SDLK_BACKSPACE ){
                    AIMode ^= 1;
                    Operation.ClearQ();
                    if( AIMode ){
                        Search();
                    }
                }
//                if( AIMode ){
//                    continue;
//                }
                switch( key ){
                    case SDLK_DOWN: AIPlayer.Operation.AddQ(MOVEDOWN);
                                    break;
                    case SDLK_LEFT: AIPlayer.Operation.AddQ(MOVELEFT);
                                    break;
                    case SDLK_RIGHT:AIPlayer.Operation.AddQ(MOVERIGHT);
                                    break;
                    case SDLK_UP  : AIPlayer.Operation.AddQ(ROTATE);
                                    break;
                    case SDLK_TAB : AIPlayer.Operation.AddQ(TURNNEXT);
                                    break;
                }
            }
        }
//        key = SDL_GetKeyState(NULL);                          //实现长按效果的第一个方法
//        if( key[SDLK_DOWN] )  Operation.AddQ(MOVEDOWN);
//        if( key[SDLK_LEFT] )  Operation.AddQ(MOVELEFT);
//        if( key[SDLK_RIGHT] ) Operation.AddQ(MOVERIGHT);
//        if( key[SDLK_UP] )    Operation.AddQ(ROTATE);

        SDL_Delay(1);                 //减轻CPU负担
    }
    SDL_KillThread(MultThread);
    SDL_KillThread(AIThread);
	pressESCtoQuit();
    return 0;
}

//-------------------------------------↑【MAIN】↑-------------------------------------------//

int GameInitial(){
    InitializeCriticalSection(&cs);
    Screen.bar(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,BLACK);
    InitBox();
    ShowBox();
    ShowScore();
    SDL_EnableKeyRepeat(400,KEYBREAK);
    ChangeTimes = 1;
    return 0;
}

void printBox(int Box[BOX_C][BOX_R]){
    int i,j;
    cout << endl << endl;
    for( i=0; i<BOX_R; i++){
        for( j=0; j<BOX_C; j++){
            cout << Box[j][i] << " ";
        }
        cout << endl;
    }
}

void xyprintf(int x,int y,const char * format, ...){
    char print_buf[1024];
    va_list args;
    int isPrinted;
    va_start(args,format);
    isPrinted = vsprintf(print_buf,format,args);
    va_end(args);
    std::cout << print_buf << std::endl;
    TextSurface text("text",print_buf,Screen,0xff,0xff,0xff);          //错因：创建text对象的时候自动执行了构造函数，读取外部字体文件失败
    text.blit(x,y);
    Screen.flip();
    return ;
}

int ShowBox(){
    int x,y;

    for( y=BY; y<=BSR+BY; y+=CS){           //横线
        Screen.line(BX,y,BX+BSC,y,DARKGRAY);
    }
    for( x=BX; x<=BSC+BX; x+=CS){           //竖线
        Screen.line(x,BY,x,BY+BSR,DARKGRAY);
    }
    Screen.rectangle(BX,BY,BX+BSC,BY+BSR,WHITE);

    for( y=BY; y<=BSR+BY; y+=CS){           //横线
        Screen.line(BX+GAP,y,BX+BSC+GAP,y,DARKGRAY);
    }
    for( x=BX+GAP; x<=BSC+BX+GAP; x+=CS){           //竖线
        Screen.line(x,BY,x,BY+BSR,DARKGRAY);
    }
    Screen.rectangle(BX+GAP,BY,BX+BSC+GAP,BY+BSR,WHITE);

    // Key Settings
    xyprintf(330,300,"ROTATE                      Up");
    xyprintf(330,320,"PAUSE                         Space");
    xyprintf(330,340,"EXIT                            Esc");
    xyprintf(330,360,"NEXT BLOCK              Tab");
    xyprintf(330,380,"RESTART                    Home");
    return 0;
}

int InitBox(){
    int i,j;
    for( i=0; i<BOX_R; i++)
        for( j=0 ;j<BOX_C; j++)
            box[j][i] = 0;
    return 0;
}

int ShowScore(){
    int completeness;                   //完成度
    xyprintf(330,480,"Score:");
//    xyprintf(330,440,"Remain Changes       %d",ChangeTimes);
    Screen.rectangle(400,480,560,500,WHITE);
    completeness = count*10 ;
//    count = 16;
    if( count>16 ) return 0;
    Screen.bar(400+1,480+1,completeness,18,GREEN);
    if( count==16 ){
//        Complete();
    }
    return 0;
}

int ShowCube(){                                     //生成一个方块的同时,在右上角显示下一个方块的形态
    int left,top;
    srand(SDL_GetTicks());
    curblk.number = next;
    next = random(CurBlkNum);
    Block nextblk = blk[next];
    nextblk.Reset(15,2);
    Screen.bar(350,100,150,200,BLACK);                           // clear previous image
    xyprintf(330,100,"Next: ");
    Screen.rectangle(350,120,500,220,WHITE);
    nextblk.DrawCube();
    // update a new block
    blk[curblk.number].Reset();                                 //恢复方块初始坐标(回到最上面)
    curblk = blk[curblk.number];
    curblk.DrawCube();
    Screen.flip();
    if( !curblk.isOk() ){                            //满了
//        Complete();
//        gameOver = true;
    }
    return 0;
}

int DelLine(){
// 从最底层往上检查消行,若此次检查消除了行返回1,否则返回0;
    int x,i,j,flag,color,left,top;
    for( i=BOX_R-1; i>0; i--){          //控制行
        flag = 1;                   //默认为满行
        for( j=0; j<10; j++){       //控制列
            if( box[j][i]==0 ){
                flag = 0;
                break;
            }
        }
        if( flag ){                 //满行,需要消除,同时更新BOX状态
            EnterCriticalSection(&cs);
            left = BX;
            top  = BY+i*CS;
            Screen.flash(left+1,top+1,BSC-2,CS-2);
            for( x=BX+CS; x<=BSC+BX; x+=CS){           //重新画竖线
                Screen.line(x,top+1,x,top+CS-2,DARKGRAY);
            }
            while( i>0 ){                              //把每一层方块往下移一层
                for( j=0; j<10; j++){
                    box[j][i] = box[j][i-1];
                    left = BX+j*CS;
                    top  = BY+i*CS;
                    color = Screen.getpixel(left+CS/2,top-CS/2);
                    Screen.bar(left+1,top+1,CS-2,CS-2,color);
                }
                i--;
            }
            LeaveCriticalSection(&cs);
            count++;
            ChangeTimes++;
            return 1;
        }
    }
    return 0;
}

int ThreadFunc(void * unused){
    int old_time,cur_time,x,y;
    old_time = cur_time = SDL_GetTicks();
    while( !gameOver ){
        while( alive ){
            while( !Operation.isEmpty() && alive){
                EnterCriticalSection(&cs);
                Operation.DeleteQ(command);
                curblk.ClearCube();
                switch( command ){
                    case MOVEDOWN: curblk.MoveDown();
                                   break;
                    case MOVELEFT: curblk.MoveLeft();
                                   break;
                    case MOVERIGHT:curblk.MoveRight();
                                   break;
                    case ROTATE  : curblk.Rotate();
                                   break;
                 }
                 if( command == TURNNEXT ){                 //有的特殊命令最好特殊处理
                    curblk.TurnNext();
                    break;
                 }
                 curblk.DrawCube();
                 Screen.flip();
                 LeaveCriticalSection(&cs);
            }
            cur_time = SDL_GetTicks();
            SDL_Delay(1);
            if( !AIMode ){
                if( cur_time - old_time < 500 ) continue;
                old_time = cur_time;
                Operation.AddQ(MOVEDOWN);
            }else{
                if( cur_time - old_time < 50 ) continue;
                old_time = cur_time;
                x = curblk.x;
                y = curblk.y;
                if( BestRotate>0 ){
                    Operation.AddQ(ROTATE);
                    BestRotate -= 1;
                    continue;
                }
                if( BestPath[x][y+1] ){
                    Operation.AddQ(MOVEDOWN);
                    BestPath[x][y+1] = 0;
                    y += 1;
                    continue;
                }else if( BestPath[x+1][y] ){
                    Operation.AddQ(MOVERIGHT);
                    BestPath[x+1][y] = 0;
                    x += 1;
                    continue;
                }else if( BestPath[x-1][y] ){
                    Operation.AddQ(MOVELEFT);
                    BestPath[x-1][y] = 0;
                    x -= 1;
                    continue;
                }
                alive = 0;
            }
        }
        while(DelLine());
        Operation.ClearQ();
        ShowCube();
        ShowScore();
        if( AIMode ) Search();
        alive = 1;
    }
    return 0;
}

void pressESCtoQuit(){
    std::cout << "pressESCtoQuit() function begin\n";
    bool gameOver = false;
    while( gameOver == false ){
        SDL_Event gameEvent;
        while ( SDL_PollEvent(&gameEvent) != 0 ){
            if ( gameEvent.type == SDL_QUIT ){
                gameOver = true;
            }
            if ( gameEvent.type == SDL_KEYUP ){
                if ( gameEvent.key.keysym.sym == SDLK_ESCAPE ){
                    gameOver = true;
                }
            }
        }
//        doSomeLoopThings();
    }
    return;
}




void Search(){
    // 根据当前的curblk的状态找出最佳移动路径
//    EnterCriticalSection(&cs);
//    cout << "isSearching" << endl;
    // Find the Top Line
    int i,j,TopLine = BOX_R;
    int path[BOX_C][BOX_R],evaluated[BOX_C][BOX_R];
    curblk.ClearCube();
    Block SearchBlk = curblk;
    BestScore = -0xff;
    BestRotate = 0;
    memset(path,0,sizeof(path));

//    path[SearchBlk.x][SearchBlk.y] = 1;
    for( i=SearchBlk.y+4; i<BOX_R; i++){
        for( j=0; j<BOX_C; j++){
            if( box[j][i] ){
                TopLine = i;
                i = BOX_R;
                break;
            }
        }
    }
    for( i=SearchBlk.y; i<TopLine-6; i++){
        path[SearchBlk.x][SearchBlk.y] = 1;
        SearchBlk.y++;
    }
    Search_start = SDL_GetTicks();
    for( i=0; i<3; i++){                         // 四种旋转变换的循环
        memset(evaluated,0,sizeof(evaluated));
        DFS(SearchBlk,path,evaluated);
        if( SearchBlk.Rotate() == -1) break;     // 无法再旋转则退出
    }
//    LeaveCriticalSection(&cs);
//    cout << "Searching Complete" << endl;
    return ;
}

void DFS(Block blk,int Path[BOX_C][BOX_R],int Evaluated[BOX_C][BOX_R]){
    int curX = blk.x, curY = blk.y;
    int path[BOX_C][BOX_R];
    memcpy(path,Path,sizeof(path));
    path[curX][curY] = 1;
//    printBox(path);
    if( SDL_GetTicks() - Search_start > 1000 ){
        AIMode = 0;
    }
    if( !blk.isOk(curX,curY+1)  ){  // && !blk.isOk(blk.x-1,blk.y) && !blk.isOk(blk.x+1,blk.y)  ?
        int score;
        if( Evaluated[blk.x][blk.y]>0 ){
            score = Evaluated[blk.x][blk.y];
        }else{
            int TestBox[BOX_C][BOX_R];
            memcpy(TestBox,box,sizeof(TestBox));
            for(int i=0; i<4; i++){
                TestBox[(curblk.x+curblk.xy[i][0])][(curblk.y+curblk.xy[i][1])] = 0;
                TestBox[(blk.x+blk.xy[i][0])][(blk.y+blk.xy[i][1])] = 1;
            }
            score = Evaluate(TestBox,path);
            Evaluated[blk.x][blk.y] = score;
        }
        if( score>BestScore ){
            BestScore = score;
            BestRotate = blk.number - curblk.number;
            if( BestRotate<0 ) BestRotate += 4;
            memcpy(BestPath,path,sizeof(BestPath));
        }
        return ;
    }
    if( blk.isOk(curX,curY+1) && !path[curX][curY+1] ){
        blk.y = curY+1;
        DFS(blk,path,Evaluated);
        blk.y = curY;
    }
    if( blk.isOk(curX-1,curY) && !path[curX-1][curY] ){
        blk.x = curX-1;
        DFS(blk,path,Evaluated);
        blk.x = curX;
    }
    if( blk.isOk(curX+1,curY) && !path[curX+1][curY] ){
        blk.x = curX+1;
        DFS(blk,path,Evaluated);
        blk.x = curX;
    }
    return ;
}

int Evaluate(int curBox[BOX_C][BOX_R],int Path[BOX_C][BOX_R]){                             // 根据当前curBox状态返回评价分数
    int i,j,k;
    int LineClear, TopLine, Holes, Canyon, Mount, Diff;
    double AvgHeight;

    // Find TopLine and Holes
    int Top, lastTop, llastTop;               //对于Top, TopLine这些值,数值越大实际的高度越低
    TopLine = BOX_R-1;
    LineClear = Holes = Diff = Canyon = Mount = 0;
    Top = lastTop = 0;
    for( i=0; i<BOX_C; i++){
        llastTop = lastTop;
        lastTop = Top;
        Top = 0;
        for( j=0; j<BOX_R; j++){
            if( curBox[i][j] ){
                Top = j;
                if( Top<TopLine ) TopLine = Top;
                AvgHeight += (BOX_R-Top);
                Diff += abs( Top-lastTop );
                break;
            }
        }
        for( ; j<BOX_R; j++){
            if( !curBox[i][j] ){
                Holes++;
            }
        }

    }
    // Find Lines Cleared and Path Length
    int flag,length = 0;
    for( j=0; j<BOX_R; j++){
        flag = 1;                   //默认满行的
        for( i=0; i<BOX_C; i++){
            if( !curBox[i][j] ){            //不是满行的
                flag = 0;
            }
            if( Path[i][j] ){
                length++;
            }
        }
        if( flag ){
            LineClear++;
        }
    }
    AvgHeight /= BOX_C;
    return TopLine*25 + LineClear*10 - Holes*40 - length/10 - AvgHeight - Diff*3;
}



int SelfMadeBlocks(int n){       //当前所画砖块在数组b中的序号(指的是大序号,如b[0]~b[3]称为1号方块
    //自定砖块功能(鼠标操作)
    // Print Grid 4*4 (260,260,340,340)
//    xyprintf(160,100,"设定一个自定义砖块(每通关一次可自选一个)");
//    xyprintf(160,140,"选择任意四个方格作为砖块的形状,选好后按space确定");
//    xyprintf(160,180,"左键单击选中，右键单击取消");

    int x,y;
    for( x=260; x<340; x+=CS ){
        Screen.line(x,260,x,340,WHITE);
    }
    for( y=260; y<340; y+=CS ){

        Screen.line(260,y,340,y,WHITE);
    }
    x = y = 260;
    Screen.rectangle(260,260,340,340,WHITE);

    int coorX,coorY;                    //当前光标的坐标
    int blkcount,gridcount;
    int i,j,TempBox[4][4];
    // Initialize Box State
    for( i=0; i<4; i++){
        for( j=0; j<4; j++){
            TempBox[i][j] = 0;
        }
    }

    // Select Grids

    coorX = coorY = 0;
    blkcount = gridcount = 0;
    int ticks = 0;

//    mouse_msg mos;
    SDL_Event Event;
    gridcount = 0;
    while( blkcount < 1 ){
        while( SDL_PollEvent(&Event) ){
            SDL_GetMouseState(&x,&y);
            if( Event.type==SDL_QUIT || Event.key.keysym.sym == SDLK_ESCAPE ){
                blkcount++;
                break;
            }
            if( Event.type==SDL_MOUSEBUTTONDOWN){
                coorX = (x-260)/CS;
                coorY = (y-260)/CS;
                if( coorX<0 || coorX>3 || coorY<0 || coorY>3 ) break;
                cout << coorX << " " << coorY << endl;
                if( gridcount < 4 ){                                //可以画
                    TempBox[coorX][coorY] ^= 1;
                    if( TempBox[coorX][coorY] ){
                        gridcount++;
                    }else{
                        gridcount--;
                    }
                }
                for(i=0; i<4; i++){
                    for(j=0; j<4; j++){
                        cout << TempBox[i][j] << " ";
                    }
                    cout << endl;
                }
                for(i=0; i<4; i++){
                    int color;
                    for(j=0; j<4; j++){
                        if( TempBox[j][i] ){
                            color = LIGHTGREEN;
                        }else{
                            color = BLACK;
                        }
                        Screen.bar(261+j*CS,261+i*CS,CS-2,CS-2,color);
                    }
                }
            }
            cout << x << "  " << y << endl;
            if( Event.type==SDL_KEYDOWN ){
                if( Event.key.keysym.sym==SDLK_SPACE && gridcount==4 ){
                    blkcount++;
                    break;
                }
            }
            Screen.flip();
        }
        SDL_Delay(100);
    }
    // Return Result
    n = n*4;
    for( i=0; i<4; i++){
        for( j=0; j<4; j++){
            b[n][i][j] = TempBox[i][j];
            b[n+1][i][3-j] = TempBox[i][j];
        }
    }
    n+=2;
    for( i=3; i>=0; i--){
        for( j=3; j>=0; j--){
            b[n][i][j] = TempBox[j][i];
            b[n+1][i][3-j] = TempBox[j][i];
        }
    }

    // Print Test
//    cleardevice();
//    n = 0;
//    for( i=0; i<4; i++){
//        for( j=0; j<4; j++){
//            xyprintf(i*20+100,j*20,"%d",b[n][i][j]);
//            xyprintf(i*20,j*20,"%d",b[n+1][i][j]);
//        }
//    }
//    getch();
//    n+=2;
//    for( i=3; i>=0; i--){
//        for( j=3; j>=0; j--){
//            xyprintf((3-i)*20+100,(3-j)*20,"%d",b[n][i][j]);
//            xyprintf((3-i)*20,(3-j)*20,"%d",b[n+1][i][j]);
//        }
//    }
//    getch();


    return 0;
}


/* ------------------------------  Block 类方法 ---------------------------------------------*/

Block::Block(){
    x = 4;                          //初始化每个方块的初始坐标
    y = 0;
    number = globalTemp;
    if( globalTemp+1<BLKNUM ) globalTemp++;
    type = number/4;
    int k = 0,i,j;
    for( i=0; i<4; i++){
        for( j=0; j<4; j++){
            if( b[number][i][j] ){
                xy[k][0] = i;
                xy[k][1] = j;
                k++;
            }
        }
    }
    switch( type ){
        case 0: color = LIGHTCYAN;        // 0~3
                break;
        case 1: color = LIGHTGREEN;        // 4~7
                break;
        case 2: color = LIGHTBLUE;        // 8~11
                break;
        case 3: color = LIGHTMAGENTA;        // 12~15
                break;
        case 4: color = LIGHTRED;        // 16~19
                break;
        case 5: color = LIGHTGRAY;        // 20~23
                break;
        case 6: color = BROWN;
                break;
        case 7: color = DARKGRAY;
                break;
        case 8: color = CYAN;
                break;
        case 9: color = WHITE;
                break;
    }
}

int Block::Reset(int newx,int newy){
    x = newx;
    y = newy;
    return 0;
}

int Block::DrawCube() const{
    int left,top,k;
    for(k=0; k<4; k++){
        if( x + xy[k][0]<10 && y + xy[k][1]<20 )box[x + xy[k][0]][y + xy[k][1]] = 1;
        left = BX+(x + xy[k][0])*CS;
        top  = BY+(y + xy[k][1])*CS;
        Screen.bar(left+1,top+1,CS-2,CS-2,color);               //+,-1是为了不画到GRID上
    }
    return 0;
}

int Block::ClearCube() const{
    int left,top,k;
    for(k=0; k<4; k++){
        box[x + xy[k][0]][y + xy[k][1]] = 0;
        left = BX+(x + xy[k][0])*CS;
        top  = BY+(y + xy[k][1])*CS;
        Screen.bar(left+1,top+1,CS-2,CS-2,BLACK);               //+,-1是为了不画到GRID上
    }
    return 0;
}

int Block::isOk() const{
    int flag = 1;               //默认可移动
    int i;
    for( i=0; i<4; i++){
        if( x + xy[i][0] >9 || x + xy[i][0] < 0 || y + xy[i][1] > 19 ){                                          //越界
            flag = 0;
            break;
        }else if( box[x + xy[i][0]][y + xy[i][1]] ){            //有障碍
            flag = 0;
            break;
        }
    }
    return flag;
}

int Block::isOk(int nextX,int nextY) const{
    int flag = 1;               //默认可移动
    int i;
    for( i=0; i<4; i++){
        if( nextX + xy[i][0] > 9 || nextX + xy[i][0] < 0 || nextY + xy[i][1] > 19 ){                                          //越界
            flag = 0;
            break;
        }else if( box[nextX + xy[i][0]][nextY + xy[i][1]] ){            //有障碍
            flag = 0;
            break;
        }
    }
    return flag;
}

int Block::MoveDown(){
    y += 1;
    if( !isOk() ){              //无法执行(到底了)
        y -= 1;
        alive = 0;
    }
    return 0;
}

int Block::MoveLeft(){
    x -= 1;
    if( !isOk() ){
        x += 1;
    }
    return 0;
}

int Block::MoveRight(){
    x += 1;
    if( !isOk() ){
        x -= 1;
    }
    return 0;
}

int Block::Rotate(){
    int DstNum = type*4 + (number+1)%4;                    //DstNum:变形后的方块的标识号(number)
    int k = 0,i,j;
    for( i=0; i<4; i++){
        for( j=0; j<4; j++){
            if( b[DstNum][i][j] ){
                xy[k][0] = i;
                xy[k][1] = j;
                k++;
            }
        }
    }
    if( isOk() ){
        number = DstNum;                   //type不用变
    }else{                              //若无法旋转，则恢复xy[][]
        k = 0;
        for( i=0; i<4; i++){
            for( j=0; j<4; j++){
                if( b[number][i][j] ){
                    xy[k][0] = i;
                    xy[k][1] = j;
                    k++;
                }
            }
        }
        return -1;
    }
//    DrawCube();
    return 0;
}

int Block::TurnNext(){
    if( ChangeTimes>0 ){
        int left,top,k;
        alive = 0;
        ChangeTimes--;
        Screen.flash(x,y,xy);
    }
    return 0;
}

/* --------------------------------------  Game 类方法  -------------------------------------------- */


int Game::AIGameInitial(){
    InitializeCriticalSection(&cs);
    AIInitBox();
    ChangeTimes = 1;
    return 0;
}

int Game::AIInitBox(){
    int i,j;
    for( i=0; i<BOX_R; i++)
        for( j=0 ;j<BOX_C; j++)
            box[j][i] = 0;
    return 0;
}


int Game::AIShowCube(){                                     //生成一个方块的同时,在右上角显示下一个方块的形态
    int left,top;
    srand(SDL_GetTicks());
    curblk.number = next;
    next = random(CurBlkNum);
    // update a new block
    blk[curblk.number].Reset();                                 //恢复方块初始坐标(回到最上面)
//    curblk = blk[curblk.number];
    curblk.x = blk[curblk.number].x;
    curblk.y = blk[curblk.number].y;
    curblk.type = blk[curblk.number].type;
    curblk.number = blk[curblk.number].number;
    curblk.color  = blk[curblk.number].color;
    memcpy(curblk.xy,blk[curblk.number].xy,sizeof(curblk.xy));

    curblk.DrawCube();
    Screen.flip();
    if( !curblk.isOk() ){                            //满了
//        Complete();
//        gameOver = true;
    }
    return 0;
}

int Game::AIDelLine(){
// 从最底层往上检查消行,若此次检查消除了行返回1,否则返回0;
    int x,i,j,flag,color,left,top;
    for( i=BOX_R-1; i>0; i--){          //控制行
        flag = 1;                   //默认为满行
        for( j=0; j<10; j++){       //控制列
            if( box[j][i]==0 ){
                flag = 0;
                break;
            }
        }
        if( flag ){                 //满行,需要消除,同时更新BOX状态
            EnterCriticalSection(&cs);
            left = BX+GAP;
            top  = BY+i*CS;
            Screen.flash(left+1,top+1,BSC-2,CS-2);
            for( x=BX+CS+GAP; x<=BSC+BX+GAP; x+=CS){           //重新画竖线
                Screen.line(x,top+1,x,top+CS-2,DARKGRAY);
            }
            while( i>0 ){                              //把每一层方块往下移一层
                for( j=0; j<10; j++){
                    box[j][i] = box[j][i-1];
                    left = BX+j*CS+GAP;
                    top  = BY+i*CS;
                    color = Screen.getpixel(left+CS/2,top-CS/2);
                    Screen.bar(left+1,top+1,CS-2,CS-2,color);
                }
                i--;
            }
            LeaveCriticalSection(&cs);
            count++;
            ChangeTimes++;
            return 1;
        }
    }
    return 0;
}

void Game::Search(){
    EnterCriticalSection(&cs);
    // Find the Top Line
    int i,j,TopLine = BOX_R;
    int path[BOX_C][BOX_R],evaluated[BOX_C][BOX_R];
    curblk.ClearCube();
    AIBlock SearchBlk = curblk;
    BestScore = -0xff;
    BestRotate = 0;
    memset(path,0,sizeof(path));

    for( i=SearchBlk.y+4; i<BOX_R; i++){
        for( j=0; j<BOX_C; j++){
            if( box[j][i] ){
                TopLine = i;
                i = BOX_R;
                break;
            }
        }
    }
    for( i=SearchBlk.y; i<TopLine-4; i++){
        path[SearchBlk.x][SearchBlk.y] = 1;
        SearchBlk.y++;
    }
    Search_start = SDL_GetTicks();
    for( i=0; i<3; i++){                         // 四种旋转变换的循环
        memset(evaluated,0,sizeof(evaluated));
        DFS(SearchBlk,path,evaluated);
        if( SearchBlk.Rotate() == -1) break;     // 无法再旋转则退出
    }
    LeaveCriticalSection(&cs);
    return ;
}

void Game::DFS(AIBlock blk,int Path[BOX_C][BOX_R],int Evaluated[BOX_C][BOX_R]){
    int curX = blk.x, curY = blk.y;
    int path[BOX_C][BOX_R];
    memcpy(path,Path,sizeof(path));
    path[curX][curY] = 1;
//    printBox(path);
    if( SDL_GetTicks() - Search_start > 3000 ){
        return ;
    }
    if( !blk.isOk(curX,curY+1)  ){  // && !blk.isOk(blk.x-1,blk.y) && !blk.isOk(blk.x+1,blk.y)  ?
        int score;
        if( Evaluated[blk.x][blk.y]>0 ){
            score = Evaluated[blk.x][blk.y];
        }else{
            int TestBox[BOX_C][BOX_R];
            memcpy(TestBox,box,sizeof(TestBox));
            for(int i=0; i<4; i++){
                TestBox[(curblk.x+curblk.xy[i][0])][(curblk.y+curblk.xy[i][1])] = 0;
                TestBox[(blk.x+blk.xy[i][0])][(blk.y+blk.xy[i][1])] = 1;
            }
            score = Evaluate(TestBox,path);
            Evaluated[blk.x][blk.y] = score;
        }
        if( score>BestScore ){
            BestScore = score;
            BestRotate = blk.number - curblk.number;
            if( BestRotate<0 ) BestRotate += 4;
            memcpy(BestPath,path,sizeof(BestPath));
        }
        return ;
    }
    if( blk.isOk(curX,curY+1) && !path[curX][curY+1] ){
        blk.y = curY+1;
        DFS(blk,path,Evaluated);
        blk.y = curY;
    }
    if( blk.isOk(curX-1,curY) && !path[curX-1][curY] ){
        blk.x = curX-1;
        DFS(blk,path,Evaluated);
        blk.x = curX;
    }
    if( blk.isOk(curX+1,curY) && !path[curX+1][curY] ){
        blk.x = curX+1;
        DFS(blk,path,Evaluated);
        blk.x = curX;
    }

}


/* ---------------------------------------  AIBlock  类方法 ---------------------------    */

AIBlock::AIBlock(){
    x = 4;                          //初始化每个方块的初始坐标
    y = 0;
    number = AIglobalTemp;
    if( AIglobalTemp+1<BLKNUM ) AIglobalTemp++;
    type = number/4;
    int k = 0,i,j;
    for( i=0; i<4; i++){
        for( j=0; j<4; j++){
            if( b[number][i][j] ){
                xy[k][0] = i;
                xy[k][1] = j;
                k++;
            }
        }
    }
    switch( type ){
        case 0: color = LIGHTCYAN;        // 0~3
                break;
        case 1: color = LIGHTGREEN;        // 4~7
                break;
        case 2: color = LIGHTBLUE;        // 8~11
                break;
        case 3: color = LIGHTMAGENTA;        // 12~15
                break;
        case 4: color = LIGHTRED;        // 16~19
                break;
        case 5: color = LIGHTGRAY;        // 20~23
                break;
        case 6: color = BROWN;
                break;
        case 7: color = DARKGRAY;
                break;
        case 8: color = CYAN;
                break;
        case 9: color = WHITE;
                break;
    }
}

int AIBlock::Reset(int newx,int newy){
    x = newx;
    y = newy;
    return 0;
}

int AIBlock::DrawCube() const{
    int left,top,k;
    for(k=0; k<4; k++){
        if( x + xy[k][0]<10 && y + xy[k][1]<20 ) AIPlayer.box[x + xy[k][0]][y + xy[k][1]] = 1;
        left = BX+(x + xy[k][0])*CS+GAP;
        top  = BY+(y + xy[k][1])*CS;
        Screen.bar(left+1,top+1,CS-2,CS-2,color);               //+,-1是为了不画到GRID上
    }
    return 0;
}

int AIBlock::ClearCube() const{
    int left,top,k;
    for(k=0; k<4; k++){
        AIPlayer.box[x + xy[k][0]][y + xy[k][1]] = 0;
        left = BX+(x + xy[k][0])*CS+GAP;
        top  = BY+(y + xy[k][1])*CS;
        Screen.bar(left+1,top+1,CS-2,CS-2,BLACK);               //+,-1是为了不画到GRID上
    }
    return 0;
}

int AIBlock::isOk() const{
    int flag = 1;               //默认可移动
    int i;
    for( i=0; i<4; i++){
        if( x + xy[i][0] >9 || x + xy[i][0] < 0 || y + xy[i][1] > 19 ){                                          //越界
            flag = 0;
            break;
        }else if( AIPlayer.box[x + xy[i][0]][y + xy[i][1]] ){            //有障碍
            flag = 0;
            break;
        }
    }
    return flag;
}

int AIBlock::isOk(int nextX,int nextY) const{
    int flag = 1;               //默认可移动
    int i;
    for( i=0; i<4; i++){
        if( nextX + xy[i][0] > 9 || nextX + xy[i][0] < 0 || nextY + xy[i][1] > 19 ){                                          //越界
            flag = 0;
            break;
        }else if( AIPlayer.box[nextX + xy[i][0]][nextY + xy[i][1]] ){            //有障碍
            flag = 0;
            break;
        }
    }
    return flag;
}

int AIBlock::MoveDown(){
    y += 1;
    if( !isOk() ){              //无法执行(到底了)
        y -= 1;
        AIPlayer.alive = 0;
    }
    return 0;
}

int AIBlock::MoveLeft(){
    x -= 1;
    if( !isOk() ){
        x += 1;
    }
    return 0;
}

int AIBlock::MoveRight(){
    x += 1;
    if( !isOk() ){
        x -= 1;
    }
    return 0;
}

int AIBlock::Rotate(){
    int DstNum = type*4 + (number+1)%4;                    //DstNum:变形后的方块的标识号(number)
    int k = 0,i,j;
    for( i=0; i<4; i++){
        for( j=0; j<4; j++){
            if( b[DstNum][i][j] ){
                xy[k][0] = i;
                xy[k][1] = j;
                k++;
            }
        }
    }
    if( isOk() ){
        number = DstNum;                   //type不用变
    }else{                              //若无法旋转，则恢复xy[][]
        k = 0;
        for( i=0; i<4; i++){
            for( j=0; j<4; j++){
                if( b[number][i][j] ){
                    xy[k][0] = i;
                    xy[k][1] = j;
                    k++;
                }
            }
        }
        return -1;
    }
    return 0;
}




