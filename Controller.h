#include "Surface.h"

/*          BOX_C = 10
     0 1 2 3 4 5 6 7 8 9
   0
   1
   2
   3
   4
   5
   6
   7
   8
   .
   .
   .

BOX_R = 20

*/

//class State{
//private:
//    int LineClear;                              // 该位置可以清除的行数
//    int Top;                                    // 该位置所属列的最高行
//    int AllTop;                                 // 当前状态的最高行
//    int Holes;                                  // 该位置所包含的洞个数(洞即一列的最高行往下的空格子)
//    int CurBox[BOX_C][BOX_R];                   // 该位置各个方格的状态
//
//public:
//    State(int box[BOX_C][BOX_R]=NULL):
//            LineClear(lineclear),Top(top),AllTop(alltop),Holes(holes) {
//        for(int i=0; i<BOX_C; i++){
//            for( int j=0; j<BOX_R; j++){
//                CurBox[i][j] = box[i][j];
//            }
//        }
//    }
//
//};

//class TestBlock: public Block{
//public:
//        bool Finished;                              // 搜索完成
//
//        TestBlock(Block CurBlk):Finished(false){}
//        void MoveDown(){
//            y += 1;
//        }
//        void MoveLeft(){
//            x -= 1;
//        }
//        void MoveRight(){
//            x += 1;
//        }
//};
