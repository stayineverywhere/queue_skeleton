# 2025-1 운영체제 과제2: Thread-safe Priority Queue

- **이름**: 윤주영  
- **학번 (마지막 3자리)**: 532  
- **제출일**: 2025-06-13 (금요일)


## 구현 기능 요약

### 핵심 기능
- **enqueue()** : key가 존재하면 value 덮어쓰기 (update) , 없으면 새 노드 삽입
- **dequeue()** : key 기준 우선순위가 가장 높은 (가장 작은) 노드 제거 및 반환
- **range(start , end)** : start ≤ key ≤ end 범위 내 노드 복사 후 새로운 큐 생성
- **깊은 복사** : 모든 반환 item/value는 깊은 복사 되어서 메인에서 해제 가능
- **thread - safe** : std::mutex 사용 , enqueue, dequeue , range에 락 적용

### 구현된 함수 목록 (`queue.cpp`)
**Queue** 연산
- queue init(void)
- void release (Queue* queue)
- Reply enqueue(Queue* queue, Item item)
- Reply dequeue(Queue* queue)
- Queue* range(Queue* queue, Key start, Key end)

**노드 관리**
- Node* nalloc(Item item)
- void nfree(Node* node)
- Node* nclone(Node* node)

### 자료구조
- **Min-heap** 기반 배열 구조로 priority queue 구현
- **Open addressing hash table**로 key → heap index 매핑 관리 (O(1) 접근)
- **Heap doubling**으로 동적 메모리 자동 확장 지원

### 동기화
- `std::mutex` + `std::lock_guard`로 쓰레드 간 경합 방지
- 다중 클라이언트 쓰레드가 동시에 `enqueue`/`dequeue` 가능

### 메모리 및 안정성
-  `value`는 `new char[]`로 깊은 복사됨 → 원본 해제해도 안전
- 모든 힙 노드는 `node_free()` 또는 `delete[]`로 명시적 해제

## 진행 상황 및 추가 설명

- 모든 요구사항(깊은 복사, update, range, thread-safety 등) 구현 완료
- range() 함수도 구현 완료 (깊은 복사 기반으로 새로운 큐 리턴)
- 성능 향상을 위한 힙 구조 및 메모리 재할당 로직 포함 (heap doubling)



