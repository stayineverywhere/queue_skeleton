#ifndef _QTYPE_H  // header guard
#define _QTYPE_H

// ==========이 파일은 수정 가능==========

typedef unsigned int Key;  // 값이 클수록 높은 우선순위
typedef void* Value;

typedef struct {
    Key key;
	void* value;  // void*로 정의, 실제 자료형은 자유롭게 변경 가능
	int value_size;  // value의 크기(바이트 단위), 필요시 사용 
} Item;

typedef struct {
    bool success;   // true: 성공, false: 실패
    Item item;
    // 필드 추가 가능
} Reply;

typedef struct node_t {
    Item item;
    struct node_t* next;
    // 필드 추가 가능
} Node;

typedef struct {
    Node* head, tail;
    // 필드 추가 가능
} Queue;

// 이후 자유롭게 추가/수정: 새로운 자료형 정의 등

#endif
#pragma once
