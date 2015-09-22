#include "Queue.h"
Queue::Queue(){
    front = rear = -1;
    number = 0;
    Lock = false;
    qArray = new ELEMENTTYPE[QUEUESIZE];
}

void Queue::AddQ(ELEMENTTYPE w){
//    if(front<0) front = 0;
    if( Lock ) return ;
    rear = rear+1;
	qArray[rear] = w;
	number+=1;
	return ;
}

void Queue::DeleteQ(ELEMENTTYPE &w){
//    if(front<0){
//        cout << "Queue Empty" << endl;
//        return ;
//    }
    if( Lock ) return ;
    front = front+1;
    w = qArray[front];
	number-=1;
	return ;
}

void Queue::ClearQ(){
    front = rear = -1;
    number = 0;
}

int Queue::NumberofQ(){
    return number;
}

int Queue::isEmpty(){
    return front==rear;
}

Queue::~Queue(){
    number = 0;
    front = rear = -1;
    delete [] qArray;
}
