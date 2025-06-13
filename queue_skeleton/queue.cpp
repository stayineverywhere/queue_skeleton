#include <iostream>
#include <mutex>
#include "queue.h"
#include <cstring>

#define INIT_CAPACITY 64
#define HASH_SIZE 10007  // Simple prime number for hash table size
#define KEY_MAP_EMPTY -1
#define KEY_MAP_DELETED -2

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

    // Key-index mapping for O(1) key lookup
    int key_map[HASH_SIZE];
};

static int hash_key(Key key) {
    return key % HASH_SIZE;
}

static void key_map_init(HeapQueue* q) {
    for (int i = 0; i < HASH_SIZE; ++i) q->key_map[i] = KEY_MAP_EMPTY;
}

static void key_map_insert(HeapQueue* q, Key key, int index) {
    int h = hash_key(key);
    while (q->key_map[h] != KEY_MAP_EMPTY && q->key_map[h] != KEY_MAP_DELETED) {
        h = (h + 1) % HASH_SIZE;
    }
    q->key_map[h] = index;
}

static int key_map_find(HeapQueue* q, Key key) {
    int h = hash_key(key);
    int tries = 0;
    while (tries++ < HASH_SIZE) {
        if (q->key_map[h] == KEY_MAP_EMPTY) return -1;
        if (q->key_map[h] >= 0 && q->heap[q->key_map[h]].key == key) return q->key_map[h];
        h = (h + 1) % HASH_SIZE;
    }
    return -1;
}

static void key_map_remove(HeapQueue* q, Key key) {
    int h = hash_key(key);
    int tries = 0;
    while (tries++ < HASH_SIZE) {
        if (q->key_map[h] == KEY_MAP_EMPTY) return;
        if (q->key_map[h] >= 0 && q->heap[q->key_map[h]].key == key) {
            q->key_map[h] = KEY_MAP_DELETED;
            return;
        }
        h = (h + 1) % HASH_SIZE;
    }
}

static void key_map_update(HeapQueue* q, Key key, int new_index) {
    int h = hash_key(key);
    int tries = 0;
    while (tries++ < HASH_SIZE) {
        if (q->key_map[h] == KEY_MAP_EMPTY) return;
        if (q->key_map[h] >= 0 && q->heap[q->key_map[h]].key == key) {
            q->key_map[h] = new_index;
            return;
        }
        h = (h + 1) % HASH_SIZE;
    }
}

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
    key_map_init(q);
    return reinterpret_cast<Queue*>(q);
}

void release(Queue* queue) {
    HeapQueue* q = reinterpret_cast<HeapQueue*>(queue);
    if (!q) return;
    for (int i = 0; i < q->size; ++i) node_free(q->heap[i]);
    delete[] q->heap;
    delete q;
}

static void heapify_up(HeapQueue* q, int idx) {
    while (idx > 0 && q->heap[parent(idx)].key > q->heap[idx].key) {
        swap(q->heap[parent(idx)], q->heap[idx]);
        key_map_update(q, q->heap[idx].key, idx);
        key_map_update(q, q->heap[parent(idx)].key, parent(idx));
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
        key_map_update(q, q->heap[idx].key, idx);
        key_map_update(q, q->heap[smallest].key, smallest);
        heapify_down(q, smallest);
    }
}

Reply enqueue(Queue* queue, Item item) {
    HeapQueue* q = reinterpret_cast<HeapQueue*>(queue);
    Reply reply = { false, {0, nullptr, 0} };
    if (!q || !item.value || item.value_size <= 0) return reply;

    std::lock_guard<std::mutex> lock(q->mtx);

    int idx = key_map_find(q, item.key);
    if (idx != -1) {
        node_free(q->heap[idx]);
        node_copy(q->heap[idx], item);
        reply.success = true;
        reply.item.key = item.key;
        reply.item.value_size = item.value_size;
        reply.item.value = new char[item.value_size];
        std::memcpy(reply.item.value, item.value, item.value_size);
        return reply;
    }

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

    HeapNode n;
    node_copy(n, item);
    q->heap[q->size] = n;
    key_map_insert(q, item.key, q->size);
    heapify_up(q, q->size);
    q->size++;

    reply.success = true;
    reply.item.key = item.key;
    reply.item.value_size = item.value_size;
    reply.item.value = new char[item.value_size];
    std::memcpy(reply.item.value, item.value, item.value_size);
    return reply;
}

Reply dequeue(Queue* queue) {
    HeapQueue* q = reinterpret_cast<HeapQueue*>(queue);
    Reply reply = { false, {0, nullptr, 0} };
    if (!q) return reply;

    std::lock_guard<std::mutex> lock(q->mtx);
    if (q->size == 0) return reply;

    HeapNode& top = q->heap[0];
    reply.success = true;
    reply.item.key = top.key;
    reply.item.value_size = top.value_size;
    reply.item.value = new char[top.value_size];
    std::memcpy(reply.item.value, top.value, top.value_size);

    key_map_remove(q, top.key);
    node_free(top);

    if (q->size > 1) {
        q->heap[0] = q->heap[q->size - 1];
        key_map_update(q, q->heap[0].key, 0);
        q->size--;
        heapify_down(q, 0);
    }
    else {
        q->size--;
    }

    return reply;
}

Queue* range(Queue* queue, Key start, Key end) {
    HeapQueue* q = reinterpret_cast<HeapQueue*>(queue);
    if (!q) return nullptr;

    HeapQueue* new_q = new HeapQueue;
    new_q->heap = new HeapNode[q->capacity];
    new_q->size = 0;
    new_q->capacity = q->capacity;
    key_map_init(new_q);

    std::lock_guard<std::mutex> lock(q->mtx);

    for (int i = 0; i < q->size; ++i) {
        if (q->heap[i].key >= start && q->heap[i].key <= end) {
            HeapNode& dest = new_q->heap[new_q->size];
            Item temp_item;
            temp_item.key = q->heap[i].key;
            temp_item.value = q->heap[i].value;
            temp_item.value_size = q->heap[i].value_size;
            node_copy(dest, temp_item);
            key_map_insert(new_q, dest.key, new_q->size);
            new_q->size++;
        }
    }

    for (int i = new_q->size / 2 - 1; i >= 0; --i) {
        heapify_down(new_q, i);
    }

    return reinterpret_cast<Queue*>(new_q);
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
