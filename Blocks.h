#ifndef BLOCKS_H
#define BLOCKS_H

#define BLKNUM 40 /*���������С(��ѡ�ķ�����̬����,��ת������µ�һ��)*/


/* ���巽��(��ת�Ĳ���һ��,��˸���״��4��)ÿ�����4*4 */

int b[BLKNUM][4][4] = {
		1, 1, 0, 0,
        1, 1, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,

        1, 1, 0, 0,
        1, 1, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,

        1, 1, 0, 0,
        1, 1, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,

        1, 1, 0, 0,
        1, 1, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
// ---------����Ϊ������,���0~3---------//

       1, 1, 1, 1,
       0, 0, 0, 0,
       0, 0, 0, 0,
       0, 0, 0, 0,

       1, 0, 0, 0,
       1, 0, 0, 0,
       1, 0, 0, 0,
       1, 0, 0, 0,

       1, 1, 1, 1,
       0, 0, 0, 0,
       0, 0, 0, 0,
       0, 0, 0, 0,

       1, 0, 0, 0,
       1, 0, 0, 0,
       1, 0, 0, 0,
       1, 0, 0, 0,

//------------����Ϊֱ��,���4~7---------------//

       1, 0, 0, 0,
       1, 1, 0, 0,
       0, 1, 0, 0,
       0, 0, 0, 0,

       0, 1, 1, 0,
       1, 1, 0, 0,
       0, 0, 0, 0,
       0, 0, 0, 0,

       1, 0, 0, 0,
       1, 1, 0, 0,
       0, 1, 0, 0,
       0, 0, 0, 0,

 	   0, 1, 1, 0,
       1, 1, 0, 0,
       0, 0, 0, 0,
       0, 0, 0, 0,
//-----------����ΪZ����A,���8~11--------------//
       0, 1, 0, 0,
       1, 1, 0, 0,
       1, 0, 0, 0,
       0, 0, 0, 0,

       1, 1, 0, 0,
       0, 1, 1, 0,
       0, 0, 0, 0,
       0, 0, 0, 0,

       0, 1, 0, 0,
       1, 1, 0, 0,
       1, 0, 0, 0,
       0, 0, 0, 0,

       1, 1, 0, 0,
       0, 1, 1, 0,
       0, 0, 0, 0,
       0, 0, 0, 0,

//------------����ΪZ����B,���12~15-------------//

       1, 1, 0, 0,
       0, 1, 0, 0,
       0, 1, 0, 0,
       0, 0, 0, 0,

       0, 0, 1, 0,
       1, 1, 1, 0,
       0, 0, 0, 0,
       0, 0, 0, 0,

       1, 0, 0, 0,
       1, 0, 0, 0,
       1, 1, 0, 0,
       0, 0, 0, 0,

       1, 1, 1, 0,
       1, 0, 0, 0,
       0, 0, 0, 0,
       0, 0, 0, 0,

//--------------����Ϊ7����A,���16~19-------------//

       1, 1, 0, 0,
       1, 0, 0, 0,
       1, 0, 0, 0,
       0, 0, 0, 0,

       1, 1, 1, 0,
       0, 0, 1, 0,
       0, 0, 0, 0,
       0, 0, 0, 0,

       0, 1, 0, 0,
       0, 1, 0, 0,
       1, 1, 0, 0,
       0, 0, 0, 0,

       1, 0, 0, 0,
       1, 1, 1, 0,
       0, 0, 0, 0,
       0, 0, 0, 0,
//--------------����Ϊ7����B,���Ϊ20~23--------//

       1, 1, 1, 0,
       0, 1, 0, 0,
       0, 0, 0, 0,
       0, 0, 0, 0,

       0, 1, 0, 0,
       1, 1, 0, 0,
       0, 1, 0, 0,
       0, 0, 0, 0,

       0, 1, 0, 0,
       1, 1, 1, 0,
       0, 0, 0, 0,
       0, 0, 0, 0,

       0, 1, 0, 0,
       0, 1, 1, 0,
       0, 1, 0, 0,
       0, 0, 0, 0,

//--------------����ΪT����,���Ϊ24~27 --------//


       1, 0, 0, 0,
       0, 1, 0, 0,
       0, 0, 1, 0,
       0, 0, 0, 1,

       0, 0, 0, 1,
       0, 0, 1, 0,
       0, 1, 0, 0,
       1, 0, 0, 0,

       1, 0, 0, 0,
       0, 1, 0, 0,
       0, 0, 1, 0,
       0, 0, 0, 1,

       0, 0, 0, 1,
       0, 0, 1, 0,
       0, 1, 0, 0,
       1, 0, 0, 0,

//--------------����Ϊб��,���Ϊ28~31 --------//



       0, 1, 0, 0,
       1, 0, 1, 0,
       0, 1, 0, 0,
       0, 0, 0, 0,

       0, 1, 0, 0,
       1, 0, 1, 0,
       0, 1, 0, 0,
       0, 0, 0, 0,

       0, 1, 0, 0,
       1, 0, 1, 0,
       0, 1, 0, 0,
       0, 0, 0, 0,

       0, 1, 0, 0,
       1, 0, 1, 0,
       0, 1, 0, 0,
       0, 0, 0, 0,
//--------------����Ϊ������,���Ϊ32~35--------//

       0, 1, 0, 1,
       1, 0, 1, 0,
       0, 0, 0, 0,
       0, 0, 0, 0,

       1, 0, 1, 0,
       0, 1, 0, 1,
       0, 0, 0, 0,
       0, 0, 0, 0,

       0, 1, 0, 0,
       0, 0, 1, 0,
       0, 1, 0, 0,
       0, 0, 1, 0,

       0, 0, 1, 0,
       0, 1, 0, 0,
       0, 0, 1, 0,
       0, 1, 0, 0,
//--------------����Ϊ������,���Ϊ36~39--------//
};


#endif // BLOCKS_H