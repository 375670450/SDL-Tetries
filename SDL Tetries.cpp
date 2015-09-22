/*
    SDL 1.2.15 Tetries(using MulThread,Transplant from EGE project)
                    By Stardust
                    2015.06
    �ؼ�: Evaluate�����ķ���ֵ
    ���⣺1.����TextSurface���text����������������text�����ʱ���Զ�ִ���˹��캯������ȡ�ⲿ�����ļ�ʧ��
          2.������ɫ��Uint32 color = 0x12345678: 12-�� 34-r 56-g 78-b ����ÿ��������һ����λʮ��������,��һ��8λ�Ķ�������
                      ���Ҫ��ĺ�ɫӦ���� r = (color>>��2��*8)&0xff ;��һ����ɫռ8λ��
          3.ö�ٶ�����Ԫ��֮��Ҫ�ö��Ÿ���
          4.��ͼ�Ĳ������ͳһֻ����һ��(��һ�����д洢Ҫ���еĲ������Ҹò���Ӧ�÷��ڷ����߳��вŲ����ӳ٣���
          5.��Ϸ����(gameOver=1)֮ǰ��Ҫ����(KillThread)�����߳�
          6.const�����ĳ�Ա���������޸ı���������ݳ�Ա��ֵ�����һ��������������֡�ͼ��
          7.GetPixel()�޷���ȷ�������أ�bpp = pScreen->format->BytesPerPixel д�� BitsPerPixel
          8.�޷�����:�������һ����Ϊֻ�е���⵽pollEvent=1�Ž����λ���жϣ��޷�ʵ�ֳ��������Ҫ��GetKeyState
                     �������������EnableKeyRepeat(int delay,int interval)���������������Ĺ��ܡ�Ч�����á�
                                 delay�ǰ���һ�����󵽿�ʼ����ظ���ͣ����ʱ��(400ms)��interval�����ζ�ȡ������Ϣ�ļ��(40ms)
          9.ƽ������
          10.AI��ʵ�֣���һ���µ����ݽṹ·��Ϊ���ģ�����A*�㷨���������·������·��ת��Ϊ����·�㣨��������ֻ��һ��������
                       ֮�������Щ·������ƶ�����
            ·�㣺Ԫ�ذ�����Ŀ�ĵ����꣬�Ⱥ��ƻ������ƣ�Ŀ�Ŀ����״
                   Ĭ�϶����Ⱥ��ƺ����ƣ��ƶ�ǰ�������״����ϣ���ƶ���ŵ�����״��������·�㣬
                   ��һ��ֻ�ƶ������Σ��ڶ����ű��Σ�Ŀ��ص���ͬ��
            ����A*�㷨�ҳ����λ��
            ˼·���������濪ʼ����(ö��λ����ɱ仯��̬�������õ�ǰ�����type���ĸ���������
          11.��ȫ�ֱ�����װ��Game����
          12.�ҵ�path���޷���ӽ�Operation: �ı�y �� path[x][y+1]��˳�����
          13.DFS�޷���x==0��λ��?
          14.AIMode�»���Ҫ��path���޷�ѡ�����ж���ز���alive=0,������ShowCube֮��ǵü� if( AIMode ) Search();
          15.AIMode����ʱ��ͻȻ��ShowCube��ʱ��ס: Search���õ�ʱ�����(��ʵ����ѭ��)
          16.���ص㡿�����޷��ƶ����������: ��ΪSearch�����������Ż�ʹ��DFS֮ǰ��ʹ�������̫��?
          17.���̺߳���ֻ����ȫ�ֺ���
*/

#include "stdlib.h"
#include "time.h"
#include "Surface.h"
#include "Blocks.h"             /*������̬����*/
#include "Queue.h"
#include "windows.h"
#include "Controller.h"

#define random(x) (rand()%x)
#define KEYBREAK 40

const int SCREEN_WIDTH =  640;
const int SCREEN_HEIGHT = 600;

enum PLAYER {

};

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



class BaseGameClass{
private:
    Block blk[BLKNUM];
    Block curblk;                     /*��ǰ�����ķ���(ʡȥ����ָ��)*/

public:
    int box[BOX_C][BOX_R];                //�ݻ�ÿ������״̬
    int Player;                           //��ʾ����Ϸ���ݶ��������1,2����AI()
    int next,nextColor;                   /*next:��һ���������ʹ���(1~BLKNUM)*/
    int alive = 0;
//    int CurBlkNum = CURBLKNUM;          /*��ǰʹ�õķ������*/
    int Search_start, Search_end;
    int BestPath[BOX_C][BOX_R];
    int BestRotate;
    int BestScore;
    int command;
    bool gameOver = false;
    Queue Operation;                      //����ִ�ж���(�Է�ͬʱִ����������)
    CRITICAL_SECTION cs;                  //�߳���
    SDL_Thread * MultThread;              //�����߳�
public:
    int ShowBox();
    int InitBox();
    int ShowCube();
    int GameInitial();
    int DelLine();
    int ShowScore();
    int ThreadFunc(void * unused);
    int Evaluate(int curBox[BOX_C][BOX_R],int Path[BOX_C][BOX_R]);
    void Search();
    template <typename BlockType>
    void DFS(BlockType blk,int Path[BOX_C][BOX_R],int Evaluated[BOX_C][BOX_R]){
        int curX = blk.x, curY = blk.y;
        int path[BOX_C][BOX_R];
        memcpy(path,Path,sizeof(path));
        path[curX][curY] = 1;
    //    printBox(path);
        if( SDL_GetTicks() - Search_start > 1000 ){
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
    //            printBox(TestBox);
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

};


class AIGameClass: public BaseGameClass{
private :
    AIBlock blk[BLKNUM];
    AIBlock curblk;
    friend class AIBlock;
public :
    int AIShowCube();
    int AIGameInitial();
    int AIDelLine();
//    void AISearch();
//    void AIDFS(AIBlock blk,int Path[BOX_C][BOX_R],int Evaluated[BOX_C][BOX_R]);
    friend int AIMain();
    friend int AIThreadFunc(void * unused);             //���̺߳���������ȫ�ֺ���
};

BaseGameClass Player;
AIGameClass AIPlayer;
ScreenSurface Screen(SCREEN_WIDTH,SCREEN_HEIGHT);
extern int b[BLKNUM][4][4];             //������̬����,��ϸ������

//int box[BOX_C][BOX_R];                  //�ݻ�ÿ������״̬
//Queue Operation;                      //����ִ�ж���(�Է�ͬʱִ����������)
//Block blk[BLKNUM];                      //��ѡ��������
//Block curblk;           /*��ǰ�����ķ���(ʡȥ����ָ��)*/
//
//int Block::globalTemp = 0;
//int AIBlock::AIglobalTemp = 0;
//int command;
//int next,nextColor; /*��ǰ�ķ������ʹ���(1~24),ֻ�����ɺ���תʱ�ı�(��ȫ�ֱ����ͱ����˸��������ݽṹ����һ����ŵ��鷳*/
//int count = 0;/*����������¼�Ѿ�����������*/
//char score[100];
//int alive = 0;
//int CurBlkNum = CURBLKNUM;          /*��ǰʹ�õķ������*/
//
//int AIMode = 0;
//int ChangeTimes;        /*���Ա任�Ĵ���*/
//bool gameOver = false;
//int Search_start, Search_end;

//SDL_Thread *MultThread = NULL;
//CRITICAL_SECTION cs;
//int BestPath[BOX_C][BOX_R];
//int BestRotate;
//int BestScore;

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



//int SelfMadeBlocks(int n);
//int Evaluate(int curBox[BOX_C][BOX_R],int Path[BOX_C][BOX_R]);
//void DFS(Block blk,int Path[BOX_C][BOX_R],int Evaluated[BOX_C][BOX_R]);
//void Search();

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
                 if( AIPlayer.command == TURNNEXT ){                 //�е���������������⴦��
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
            old_time = cur_time;
            x = AIPlayer.curblk.x;
            y = AIPlayer.curblk.y;
            if( AIPlayer.BestRotate>0 ){
                AIPlayer.Operation.AddQ(ROTATE);
                AIPlayer.BestRotate -= 1;
                continue;
            }
            if( AIPlayer.BestPath[x][y+1] ){
                AIPlayer.Operation.AddQ(MOVEDOWN);
                AIPlayer.BestPath[x][y+1] = 0;
                y += 1;
                continue;
            }else if( AIPlayer.BestPath[x+1][y] ){
                AIPlayer.Operation.AddQ(MOVERIGHT);
                AIPlayer.BestPath[x+1][y] = 0;
                x += 1;
                continue;
            }else if( AIPlayer.BestPath[x-1][y] ){
                AIPlayer.Operation.AddQ(MOVELEFT);
                AIPlayer.BestPath[x-1][y] = 0;
                x -= 1;
                continue;
            }
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
//    AIPlayer.AIGameInitial();
    AIPlayer.alive = 0;
    AIPlayer.gameOver = false;
    AIPlayer.Search_start = 0, AIPlayer.Search_end=0;

    AIPlayer.MultThread = SDL_CreateThread(ThreadFunc,NULL);
    while( AIPlayer.gameOver == false ){
        SDL_Delay(100);                 //����CPU����
    }
    SDL_KillThread(AIPlayer.MultThread);
    return 0;
}

//-------------------------------------����MAIN����-------------------------------------------//

int main(int argc ,char* argv[]){
//    freopen( "CON", "wt", stdout );
//    SelfMadeBlocks(0);
    GameInitial();
    Sint32 key;
    SDL_Event gameEvent;
    Player.MultThread = SDL_CreateThread(ThreadFunc,NULL);
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
                        Player.Search();
                    }
                }
                if( AIMode ){
                    continue;
                }
                switch( key ){
                    case SDLK_DOWN: Operation.AddQ(MOVEDOWN);
                                    break;
                    case SDLK_LEFT: Operation.AddQ(MOVELEFT);
                                    break;
                    case SDLK_RIGHT:Operation.AddQ(MOVERIGHT);
                                    break;
                    case SDLK_UP  : Operation.AddQ(ROTATE);
                                    break;
                    case SDLK_TAB : Operation.AddQ(TURNNEXT);
                                    break;
                }
            }
        }
//        key = SDL_GetKeyState(NULL);                          //ʵ�ֳ���Ч���ĵ�һ������
//        if( key[SDLK_DOWN] )  Operation.AddQ(MOVEDOWN);
//        if( key[SDLK_LEFT] )  Operation.AddQ(MOVELEFT);
//        if( key[SDLK_RIGHT] ) Operation.AddQ(MOVERIGHT);
//        if( key[SDLK_UP] )    Operation.AddQ(ROTATE);

        SDL_Delay(1);                 //����CPU����
    }
    SDL_KillThread(MultThread);
	pressESCtoQuit();
    return 0;
}

//-------------------------------------����MAIN����-------------------------------------------//

int BaseGameClass::GameInitial(){
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
    TextSurface text("text",print_buf,Screen,0xff,0xff,0xff);          //���򣺴���text�����ʱ���Զ�ִ���˹��캯������ȡ�ⲿ�����ļ�ʧ��
    text.blit(x,y);
    Screen.flip();
    return ;
}

int BaseGameClass::ShowBox(){
    int x,y;

    for( y=BY; y<=BSR+BY; y+=CS){           //����
        Screen.line(BX,y,BX+BSC,y,DARKGRAY);
    }
    for( x=BX; x<=BSC+BX; x+=CS){           //����
        Screen.line(x,BY,x,BY+BSR,DARKGRAY);
    }
    Screen.rectangle(BX,BY,BX+BSC,BY+BSR,WHITE);
    // Key Settings
    xyprintf(330,300,"ROTATE                      Up");
    xyprintf(330,320,"PAUSE                         Space");
    xyprintf(330,340,"EXIT                            Esc");
    xyprintf(330,360,"NEXT BLOCK              Tab");
    xyprintf(330,380,"RESTART                    Home");
    return 0;
}

int BaseGameClass::InitBox(){
    int i,j;
    for( i=0; i<BOX_R; i++)
        for( j=0 ;j<BOX_C; j++)
            box[j][i] = 0;
    return 0;
}

int BaseGameClass::ShowScore(){
    int completeness;                   //��ɶ�
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

int BaseGameClass::ShowCube(){                                     //����һ�������ͬʱ,�����Ͻ���ʾ��һ���������̬
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
    blk[curblk.number].Reset();                                 //�ָ������ʼ����(�ص�������)
    curblk = blk[curblk.number];
    curblk.DrawCube();
    Screen.flip();
    if( !curblk.isOk() ){                            //����
//        Complete();
        gameOver = false;
    }
    return 0;
}

int BaseGameClass::DelLine(){
// ����ײ����ϼ������,���˴μ���������з���1,���򷵻�0;
    int x,i,j,flag,color,left,top;
    for( i=BOX_R-1; i>0; i--){          //������
        flag = 1;                   //Ĭ��Ϊ����
        for( j=0; j<10; j++){       //������
            if( box[j][i]==0 ){
                flag = 0;
                break;
            }
        }
        if( flag ){                 //����,��Ҫ����,ͬʱ����BOX״̬
            EnterCriticalSection(&cs);
            left = BX;
            top  = BY+i*CS;
            Screen.flash(left+1,top+1,BSC-2,CS-2);
            for( x=BX+CS; x<=BSC+BX; x+=CS){           //���»�����
                Screen.line(x,top+1,x,top+CS-2,DARKGRAY);
            }
            while( i>0 ){                              //��ÿһ�㷽��������һ��
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

int BaseGameClass::ThreadFunc(void * unused){
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
                 if( command == TURNNEXT ){                 //�е���������������⴦��
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

//void BaseGameClass::DFS(Block blk,int Path[BOX_C][BOX_R],int Evaluated[BOX_C][BOX_R]){
//
//}

void BaseGameClass::Search(){
    // ���ݵ�ǰ��curblk��״̬�ҳ�����ƶ�·��
    EnterCriticalSection(&cs);
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
    for( i=0; i<3; i++){                         // ������ת�任��ѭ��
        memset(evaluated,0,sizeof(evaluated));
        DFS(SearchBlk,path,evaluated);
        if( SearchBlk.Rotate() == -1) break;     // �޷�����ת���˳�
    }
    LeaveCriticalSection(&cs);
//    cout << "Searching Complete" << endl;
    return ;
}

int BaseGameClass::Evaluate(int curBox[BOX_C][BOX_R],int Path[BOX_C][BOX_R]){                             // ���ݵ�ǰcurBox״̬�������۷���
    int i,j,k;
    int LineClear, TopLine, Holes, Canyon, Mount, Diff;
    double AvgHeight;
//    printBox(curBox);
    // Find TopLine and Holes
    int Top, lastTop, llastTop;               //����Top, TopLine��Щֵ,��ֵԽ��ʵ�ʵĸ߶�Խ��
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
        flag = 1;                   //Ĭ�����е�
        for( i=0; i<BOX_C; i++){
            if( !curBox[i][j] ){            //�������е�
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


int AIGameClass::AIShowCube(){

}
int AIGameClass::AIGameInitial(){

}
int AIGameClass::AIDelLine(){

}

/* ------------------------------  Block �෽�� ---------------------------------------------*/

Block::Block(){
    x = 4;                          //��ʼ��ÿ������ĳ�ʼ����
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
        Screen.bar(left+1,top+1,CS-2,CS-2,color);               //+,-1��Ϊ�˲�����GRID��
    }
    return 0;
}

int Block::ClearCube() const{
    int left,top,k;
    for(k=0; k<4; k++){
        box[x + xy[k][0]][y + xy[k][1]] = 0;
        left = BX+(x + xy[k][0])*CS;
        top  = BY+(y + xy[k][1])*CS;
        Screen.bar(left+1,top+1,CS-2,CS-2,BLACK);               //+,-1��Ϊ�˲�����GRID��
    }
    return 0;
}

int Block::isOk() const{
    int flag = 1;               //Ĭ�Ͽ��ƶ�
    int i;
    for( i=0; i<4; i++){
        if( x + xy[i][0] >9 || x + xy[i][0] < 0 || y + xy[i][1] > 19 ){                                          //Խ��
            flag = 0;
            break;
        }else if( box[x + xy[i][0]][y + xy[i][1]] ){            //���ϰ�
            flag = 0;
            break;
        }
    }
    return flag;
}

int Block::isOk(int nextX,int nextY) const{
    int flag = 1;               //Ĭ�Ͽ��ƶ�
    int i;
    for( i=0; i<4; i++){
        if( nextX + xy[i][0] > 9 || nextX + xy[i][0] < 0 || nextY + xy[i][1] > 19 ){                                          //Խ��
            flag = 0;
            break;
        }else if( box[nextX + xy[i][0]][nextY + xy[i][1]] ){            //���ϰ�
            flag = 0;
            break;
        }
    }
    return flag;
}

int Block::MoveDown(){
    y += 1;
    if( !isOk() ){              //�޷�ִ��(������)
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
    int DstNum = type*4 + (number+1)%4;                    //DstNum:���κ�ķ���ı�ʶ��(number)
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
        number = DstNum;                   //type���ñ�
    }else{                              //���޷���ת����ָ�xy[][]
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

/* ---------------------------------------  AIBlock  �෽�� ---------------------------    */

AIBlock::AIBlock(){
    x = 4;                          //��ʼ��ÿ������ĳ�ʼ����
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
        Screen.bar(left+1,top+1,CS-2,CS-2,color);               //+,-1��Ϊ�˲�����GRID��
    }
    return 0;
}

int AIBlock::ClearCube() const{
    int left,top,k;
    for(k=0; k<4; k++){
        AIPlayer.box[x + xy[k][0]][y + xy[k][1]] = 0;
        left = BX+(x + xy[k][0])*CS+GAP;
        top  = BY+(y + xy[k][1])*CS;
        Screen.bar(left+1,top+1,CS-2,CS-2,BLACK);               //+,-1��Ϊ�˲�����GRID��
    }
    return 0;
}

int AIBlock::isOk() const{
    int flag = 1;               //Ĭ�Ͽ��ƶ�
    int i;
    for( i=0; i<4; i++){
        if( x + xy[i][0] >9 || x + xy[i][0] < 0 || y + xy[i][1] > 19 ){                                          //Խ��
            flag = 0;
            break;
        }else if( AIPlayer.box[x + xy[i][0]][y + xy[i][1]] ){            //���ϰ�
            flag = 0;
            break;
        }
    }
    return flag;
}

int AIBlock::isOk(int nextX,int nextY) const{
    int flag = 1;               //Ĭ�Ͽ��ƶ�
    int i;
    for( i=0; i<4; i++){
        if( nextX + xy[i][0] > 9 || nextX + xy[i][0] < 0 || nextY + xy[i][1] > 19 ){                                          //Խ��
            flag = 0;
            break;
        }else if( AIPlayer.box[nextX + xy[i][0]][nextY + xy[i][1]] ){            //���ϰ�
            flag = 0;
            break;
        }
    }
    return flag;
}

int AIBlock::MoveDown(){
    y += 1;
    if( !isOk() ){              //�޷�ִ��(������)
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
    int DstNum = type*4 + (number+1)%4;                    //DstNum:���κ�ķ���ı�ʶ��(number)
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
        number = DstNum;                   //type���ñ�
    }else{                              //���޷���ת����ָ�xy[][]
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

