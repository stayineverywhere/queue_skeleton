#include <iostream>
#include <mutex>
#include "queue.h"
#include <cstring>

#define INIT_CAPACITY 64 // 초기 큐 용량

struct HeapNode {
	Key key;
	void* value;
	int value_size;
};

struct HeapQueue { 
	HeapNode* heap;
	int size;
	int capacity;
	std::mutex mtx;
};


static void node_free(HeapNode& node) {
	if (node.value) {
		delete[] static_cast<char*>(node.value);
		node.value = nullptr;
		node.value_size = 0;
	}
}

static void node_copy(HeapNode& dst, const Item& src) {
	dst.key = src.key;
	dst.value_size = src.value_size;
	dst.value = new char[src.value_size];
	std::memcpy(dst.value, src.value, src.value_size);
}


static void swap(HeapNode& a, HeapNode& b) {
	HeapNode tmp = a;
	a = b;
	b = tmp;
}

static int parent(int i) { return (i - 1) / 2; }
static int left(int i) { return 2 * i + 1; }
static int right(int i) { return 2 * i + 2; }

Queue* init(void) {
	HeapQueue* q = new HeapQueue;
	q->heap = new HeapNode[INIT_CAPACITY];
	q->size = 0;
	q->capacity = INIT_CAPACITY;
	return reinterpret_cast<Queue*>(q); // 타입 캐스팅
}


void release(Queue* queue) {
	HeapQueue* q = reinterpret_cast<HeapQueue*>(queue);
	if (!q) return;
	for (int i = 0; i < q->size; ++i) node_free(q->heap[i]);
	delete[] q->heap;
	delete q;
}

static int find_key(HeapQueue* q, Key key) {
	for (int i = 0; i < q->size; ++i) {
		if (q->heap[i].key == key) return i;
	}
	return -1;
}


static void heapify_up(HeapQueue* q, int idx) {
	while (idx > 0 && q->heap[parent(idx)].key > q->heap[idx].key) {
		swap(q->heap[parent(idx)], q->heap[idx]);
		idx = parent(idx);
	}
}


static void heapify_down(HeapQueue* q, int idx) {
	int smallest = idx;
	int l = left(idx), r = right(idx);
	if (l < q->size && q->heap[l].key < q->heap[smallest].key) smallest = l;
	if (r < q->size && q->heap[r].key < q->heap[smallest].key) smallest = r;
	if (smallest != idx) {
		swap(q->heap[smallest], q->heap[idx]);
		heapify_down(q, smallest);
	}
}

Node* nalloc(Item item) {
	Node* node = new Node;
	node->item.key = item.key;
	node->item.value_size = item.value_size;
	node->item.value = new char[item.value_size];
	std::memcpy(node->item.value, item.value, item.value_size);
	node->next = nullptr;
	return node;
}


void nfree(Node* node) {
	if (!node) return;
	if (node->item.value) {
		delete[] static_cast<char*>(node->item.value);
		node->item.value = nullptr;
	}
	delete node;
}


Node* nclone(Node* node) {
	if (!node) return nullptr;
	return nalloc(node->item);
}


Reply enqueue(Queue* queue, Item item) {
	HeapQueue* q = reinterpret_cast<HeapQueue*>(queue);
	Reply reply;
	reply.success = false;
	reply.item.key = 0;
	reply.item.value = nullptr;
	reply.item.value_size = 0;
	if (!q || !item.value || item.value_size <= 0) return reply;

	std::lock_guard<std::mutex> lock(q->mtx);

	int idx = find_key(q, item.key);
	if (idx != -1) {
		// key 중복: value만 교체(깊은 복사)
		node_free(q->heap[idx]);
		node_copy(q->heap[idx], item);
		reply.success = true;
		reply.item = item;
		return reply;
	}

	// 용량 부족시 doubling
	if (q->size == q->capacity) {
		int newcap = q->capacity * 2;
		HeapNode* newheap = new HeapNode[newcap];
		for (int i = 0; i < q->size; ++i) {
			node_copy(newheap[i], { q->heap[i].key, q->heap[i].value, q->heap[i].value_size });
			node_free(q->heap[i]);
		}
		delete[] q->heap;
		q->heap = newheap;
		q->capacity = newcap;
	}

	// 새 노드 삽입
	HeapNode n;
	node_copy(n, item);
	q->heap[q->size] = n;
	heapify_up(q, q->size);
	q->size++;

	reply.success = true;
	reply.item = item;
	return reply;
}



Reply dequeue(Queue* queue) {
	HeapQueue* q = reinterpret_cast<HeapQueue*>(queue);
	Reply reply;
	reply.success = false;
	reply.item.key = 0;
	reply.item.value = nullptr;
	reply.item.value_size = 0;
	if (!q) return reply;

	std::lock_guard<std::mutex> lock(q->mtx);

	if (q->size == 0) return reply;

	HeapNode& top = q->heap[0];
	// 반환용 깊은 복사
	Item ret;
	ret.key = top.key;
	ret.value_size = top.value_size;
	ret.value = new char[top.value_size];
	std::memcpy(ret.value, top.value, top.value_size);

	reply.success = true;
	reply.item = ret;

	// 힙에서 제거
	node_free(top);
	if (q->size > 1) {
		q->heap[0] = q->heap[q->size - 1];
		heapify_down(q, 0);
	}
	q->size--;

	return reply;
}


Queue* range(Queue* queue, Key start, Key end) {
	return nullptr;
}
