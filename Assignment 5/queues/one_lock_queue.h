#include "../common/allocator.h"
#include <mutex>

template <class T>
class Node {
public:
	T value;
	Node <T>* next;
};

template <class T>
class OneLockQueue {
	Node <T>* head;
	Node <T>* tail;
	std::mutex lock;
	CustomAllocator my_allocator_;
public:
	OneLockQueue() : my_allocator_() {
		std::cout << "Using OneLockQueue\n";
	}

	void initQueue(long t_my_allocator_size){
		std::cout << "Using Allocator\n";
		my_allocator_.initialize(t_my_allocator_size, sizeof(Node<T>));
		// Initialize the queue head or tail here
		Node<T>* newNode = (Node<T>*)my_allocator_.newNode();
		head = newNode;
		tail = newNode;
		my_allocator_.freeNode(newNode);
	}

	void enqueue(T value) {
		Node <T>* newNode = (Node <T>*) my_allocator_.newNode();
        newNode->value = value;
        newNode->next = NULL;

		lock.lock();
		tail->next = newNode;
		tail = newNode;
		my_allocator_.freeNode(newNode);
		lock.unlock();
	}

	bool dequeue(T *value) {
		bool ret_value = false;
		lock.lock();

		Node <T>* node = head;
		Node <T>* new_head = head->next;

        if(new_head == NULL) {
			ret_value = false;
        }
		else {
			*value = new_head->value;
			head = new_head;
			ret_value = true;
		}

		my_allocator_.freeNode(node);
		lock.unlock();
        return ret_value;
	}

	void cleanup() {
		my_allocator_.cleanup();
	}
};