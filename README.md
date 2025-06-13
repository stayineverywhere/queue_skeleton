# 2025-1 운영체제 과제2: Thread-safe Priority Queue

- **이름**: 윤주영  
- **학번 (마지막 3자리)**: 532  
- **제출일**: 2025-06-13 (금요일)


## 구현 내용

### 핵심 기능
- **Thread-safe한 Priority Queue** 구현 (min-heap 구조 기반)
- 동시 다중 클라이언트 환경에서 안전하게 `enqueue`, `dequeue`, `range` 수행 가능
- `std::mutex`를 이용한 락 기반 동기화

### 구현된 함수 목록 (`queue.cpp`)
- `Queue* init(void)`
- `void release(Queue* queue)`
- `Reply enqueue(Queue* queue, Item item)`
  - 기존 key가 존재하면 **value를 덮어씀 (update)**
  - `Reply.item`은 **깊은 복사**되어 반환
- `Reply dequeue(Queue* queue)`
  - 우선순위가 가장 높은(=key가 가장 작은) item을 제거 후 반환
- `Queue* range(Queue* queue, Key start, Key end)`
  - 주어진 key 범위에 해당하는 노드들을 **복제한 새로운 큐** 생성

### 메모리 및 안정성
- 모든 동적 메모리(`value`, `heap`)는 `new/delete[]`를 통해 수동 관리
- 리턴된 `Reply.item.value`는 메인에서 해제해도 안전한 **깊은 복사 구조**
- `enqueue()`와 `dequeue()` 중간에 context switch 발생해도 무결성 유지


## 사용 제한 / 주의사항

- STL 컨테이너 (`vector`, `map` 등) **미사용**
- `qtype.h`, `queue.cpp`만 수정하였으며 `queue.h`, `main.c`는 변경 없음
- 테스트 코드는 `main.c` 기반 단일 클라이언트 테스트로 작성하였으며, 멀티스레드 환경도 안전함

## 진행 상황 및 추가 설명

- 모든 요구사항(깊은 복사, update, range, thread-safety 등) 구현 완료
- range() 함수도 구현 완료 (깊은 복사 기반으로 새로운 큐 리턴)
- 성능 향상을 위한 힙 구조 및 메모리 재할당 로직 포함 (heap doubling)



