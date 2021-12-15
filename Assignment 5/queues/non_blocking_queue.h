#include "../common/allocator.h"
#include "../common/utils.h"

#define LFENCE asm volatile("lfence" : : : "memory")
#define SFENCE asm volatile("sfence" : : : "memory")
#define ADDRESS_MASK 0x0000ffffffffffff
#define ADD_MASK 0x0001000000000000
#define ADDRESS_LEN 48

template <class P>
struct pointer_t {
	P* ptr;

	P* address() {
		return (P *) ((long long int)ptr & ADDRESS_MASK);
	}
	uint count() {
		return ((uint)(((long long int)ptr & ~ADDRESS_MASK) >> ADDRESS_LEN));
	}
	void increment(){
		ptr = (P *) ((((long long int) ptr & ADDRESS_MASK)) ^ ADD_MASK);
	}
	bool operator == (const pointer_t &in) const {
		return in.ptr == ptr;
	}
};

template <class T>
class Node {
public:
	T value;
	pointer_t <Node<T>> next;
};

template <class T>
class NonBlockingQueue {
	pointer_t <Node<T>> head;
	pointer_t <Node<T>> tail;
	CustomAllocator my_allocator_;
public:
	NonBlockingQueue() : my_allocator_() {
		std::cout << "Using NonBlockingQueue\n";
	}

	void initQueue(long t_my_allocator_size) {
		std::cout << "Using Allocator\n";
		my_allocator_.initialize(t_my_allocator_size, sizeof(Node<T>));
		// Initialize the queue head or tail here
		Node<T>* newNode = (Node<T>*)my_allocator_.newNode();
		newNode->next.ptr = NULL;
		head.ptr = tail.ptr = newNode;
		my_allocator_.freeNode(newNode);
	}

	void enqueue(T value) {
		Node<T>* node = (Node<T>* )my_allocator_.newNode();
		node->value = value;
		node->next.ptr = NULL;
		SFENCE;
		pointer_t <Node<T>> last;
		pointer_t <Node<T>> next;
		while(true) {
			last = tail;
			LFENCE;
			next = last.address()->next;
			LFENCE;
			if (last == tail){
				if (next.address() == NULL) {
					if(CAS(&last.address()->next, next, pointer_t <Node<T>>{node})){
						next.increment();
						break;
					}
				}
				else{
					if(CAS(&tail, last, pointer_t <Node<T>> {next.address()}))
						last.increment();
				}
			}
		}
		SFENCE;
		if(CAS(&tail, last, pointer_t<Node<T>> {node}))
			last.increment();
	}

	bool dequeue(T *value) {
		pointer_t <Node<T>> first;
		pointer_t <Node<T>> last;
		pointer_t <Node<T>> next;

		while(true){
			first = head;
			LFENCE;
			last = tail;
			LFENCE;
			next = first.address()->next;
			LFENCE;
			if (first == head) {
				if(first.address() == last.address()) {
					if(next.address() == NULL)
						return false;
					if(CAS(&tail, last, pointer_t <Node<T>>{next.address()}))
						last.increment();
				}
				else {
					*value = next.address()->value;
					if(CAS(&head, first, pointer_t <Node<T>>{next.address()})) {
						first.increment();
						break;
					}
				}
			}
		}
		my_allocator_.freeNode(first.address());
		return true;
	}

	void cleanup() {
		my_allocator_.cleanup();
	}
};