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

struct Queue {
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
	Queue* q = new Queue;
	q->heap = new HeapNode[INIT_CAPACITY];
	q->size = 0;
	q->capacity = INIT_CAPACITY;
	return q;
}


void release(Queue* queue) {
	if (!queue) return;
	for (int i = 0; i < queue->size; ++i) {
		node_free(queue->heap[i]);
	}
	delete[] queue->heap;
	delete queue;
}

static int find_key(Queue* q, Key key) {
	for (int i = 0; i < q->size; ++i) {
		if (q->heap[i].key == key) return i;
	}
	return -1;
}


static void heapify_up(Queue* q, int idx) {
	while (idx > 0 && q->heap[parent(idx)].key > q->heap[idx].key) {
		swap(q->heap[parent(idx)], q->heap[idx]);
		idx = parent(idx);
	}
}


static void heapify_down(Queue* q, int idx) {
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
	Reply reply = { false, {0, nullptr, 0} };
	if (!queue || !item.value || item.value_size <= 0) return reply;

	std::lock_guard<std::mutex> lock(queue->mtx);

	int idx = find_key(queue, item.key);
	if (idx != -1) {
		// key 중복: value만 교체(깊은 복사)
		node_free(queue->heap[idx]);
		node_copy(queue->heap[idx], item);
		reply.success = true;
		reply.item = item;
		return reply;
	}

	// 용량 부족시 doubling
	if (queue->size == queue->capacity) {
		int newcap = queue->capacity * 2;
		HeapNode* newheap = new HeapNode[newcap];
		for (int i = 0; i < queue->size; ++i) {
			node_copy(newheap[i], { queue->heap[i].key, queue->heap[i].value, queue->heap[i].value_size });
			node_free(queue->heap[i]);
		}
		delete[] queue->heap;
		queue->heap = newheap;
		queue->capacity = newcap;
	}

	// 새 노드 삽입
	HeapNode n;
	node_copy(n, item);
	queue->heap[queue->size] = n;
	heapify_up(queue, queue->size);
	queue->size++;

	reply.success = true;
	reply.item = item;
	return reply;
}


Reply dequeue(Queue* queue) {
	Reply reply = { false, {0, nullptr, 0} };
	if (!queue) return reply;

	std::lock_guard<std::mutex> lock(queue->mtx);

	if (queue->size == 0) return reply;

	HeapNode& top = queue->heap[0];
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
	if (queue->size > 1) {
		queue->heap[0] = queue->heap[queue->size - 1];
		heapify_down(queue, 0);
	}
	queue->size--;

	return reply;
}

Queue* range(Queue* queue, Key start, Key end) {
	return nullptr;
}
