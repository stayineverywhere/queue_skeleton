#ifndef _QUEUE_H // header guard
#define _QUEUE_H 

// ==========�� ������ ���� �Ұ�==========
// ����: �Ϲ������� queue_init()�� ���� prefix�� �ٿ� �Ҽ�(?)�� ��Ȯ�� ��
// �� ���������� ���Ǹ� ���� ª�� �̸��� ���

#include "qtype.h"

// ť �ʱ�ȭ, ����
Queue* init(void);
void release(Queue* queue);


// ==========concurrent operations==========

// ��� ����&�ʱ�ȭ, ����, ����
Node* nalloc(Item item);
void nfree(Node* node);
Node* nclone(Node* node);

// (key, item)�� Ű���� ���� ������ ��ġ�� �߰�
// Reply: ������ ������ success=true, �ƴϸ� success=false
Reply enqueue(Queue* queue, Item item);

//  ù ��° ���(Ű���� ���� ū ���)�� ����(ť���� �����ϰ� �������� ����)
// Reply: ť�� ��������� success=false
//        �ƴϸ�          success=true,  item=ù ��° ����� ������
Reply dequeue(Queue* queue);

// start <= key && key <= end �� ������ ã��,
// �ش� ������ <����>�ؼ� ���ο� ť�� ������ �� �����͸� ����
Queue* range(Queue* queue, Key start, Key end);

#endif
#pragma once
