#ifndef QUEUE_H
#define QUEUE_H

#define QUEUESIZE 99999
#define LISTSIZE 36
#define ELEMENTTYPE int
using namespace std;

//struct QueueNode{
//	int x;
//	int y;
//	int Distance;
//};


class Queue{
public:
        int rear;
        int front;
        int number;
        ELEMENTTYPE * qArray ;
        bool Lock;
public:
        Queue();
        void AddQ(ELEMENTTYPE w);
        void DeleteQ(ELEMENTTYPE &w);
        void ClearQ();
        int NumberofQ();
        int isEmpty();
        ~Queue();
};
#endif // QUEUE_H

