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
//    int LineClear;                              // ��λ�ÿ������������
//    int Top;                                    // ��λ�������е������
//    int AllTop;                                 // ��ǰ״̬�������
//    int Holes;                                  // ��λ���������Ķ�����(����һ�е���������µĿո���)
//    int CurBox[BOX_C][BOX_R];                   // ��λ�ø��������״̬
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
//        bool Finished;                              // �������
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
