/****************************************************************
 * Purpose: This container used as replacement for stdlib one to*
 * avoid dynamic memory allocation on embedded microcontrollers	*
 * but it keeps C++ STL like containers style author aware about*
 * allocattors and other ways to achive this container logic.	*
 * it was done for learning mostly, also to provide ligh 	*
 * weight, easy modificable lib, portable across own projects.	*
 ****************************************************************
 * Author: Pavel Ruban | 19.06.2016 http://pavelruban.org	*
 ****************************************************************/

#define QUEUE_FULL 1
#define QUEUE_IS_NOT_FULL 0

template <typename T, uint16_t qsize>
class Queue
{
private:
	T array[qsize];

public:
	uint16_t start_index;
	uint16_t end_index;
	uint8_t full;

	Queue();
	~Queue();

	uint16_t size();
	uint16_t capacity();
	uint8_t empty();
	void clear();

	T * data();

	uint8_t push_back(T const&);
	// uint8_t push_front(T const&);

	void pop_front();
	//T pop_back();


	T & front();
	T & back();

	class iterator
	{
	private:
		Queue<T, qsize> *queue;
		uint16_t index;
	public:
		iterator(Queue *, uint16_t index);
		~iterator();

		void operator++();

		T * operator->();
		T & operator*();
		uint8_t operator!=(iterator it);
		uint8_t operator==(iterator it);
	};

	Queue<T, qsize>::iterator begin();
	Queue<T, qsize>::iterator end();
};

///////////////////////// QUEUE::ITERATOR CONSTRUCTORS ///////////////////////////////
template <typename T, uint16_t qsize>
Queue<T, qsize>::iterator::iterator(Queue<T, qsize> *queue, uint16_t index)
{
	this->queue = queue;
	this->index = index;
}

template <typename T, uint16_t qsize>
Queue<T, qsize>::iterator::~iterator() {}

template <typename T, uint16_t qsize>
typename Queue<T, qsize>::iterator Queue<T, qsize>::begin()
{
	return iterator(this, start_index);
}

template <typename T, uint16_t qsize>
typename Queue<T, qsize>::iterator Queue<T, qsize>::end()
{
	return iterator(this, end_index);
}

///////////////////////// ITERATOR OPERATORS ///////////////////////////////
template <typename T, uint16_t qsize>
void Queue<T, qsize>::iterator::operator++()
{
	if (queue->empty()) return;

	++index;

	// If we reach the end of queue, but end_index still not reached, it
	// means queue had free space at begining and end_index there, as end
	// was moved to first hald to fill the empty space with needed data.
	if (index >= qsize && index != queue->end_index)
	{
		index = 0;
	}
}

template <typename T, uint16_t qsize>
uint8_t Queue<T, qsize>::iterator::operator!=(iterator it)
{
	return it.index != this->index;
}

template <typename T, uint16_t qsize>
T * Queue<T, qsize>::iterator::operator->()
{
	return &(this->queue->array[index]);
}

template <typename T, uint16_t qsize>
T & Queue<T, qsize>::iterator::operator*()
{
	return this->queue->array[index];
}

template <typename T, uint16_t qsize>
uint8_t Queue<T, qsize>::iterator::operator==(iterator it)
{
	return it.index == this->index;
}

///////////////////////// QUEUE CONSTRUCTORS AND METHODS ///////////////////////////////
template <typename T, uint16_t qsize>
Queue<T, qsize>::Queue()
{
	full = QUEUE_IS_NOT_FULL;
	start_index = end_index = 0;
}

template <typename T, uint16_t qsize>
Queue<T, qsize>::~Queue() {}

template <typename T, uint16_t qsize>
uint16_t Queue<T, qsize>::size()
{
	return end_index - start_index;
}

template <typename T, uint16_t qsize>
uint16_t Queue<T, qsize>::capacity()
{
	return qsize;
}

template <typename T, uint16_t qsize>
uint8_t Queue<T, qsize>::empty()
{
	return end_index == start_index;
}

template <typename T, uint16_t qsize>
void Queue<T, qsize>::clear()
{
	full = QUEUE_IS_NOT_FULL;
	start_index = end_index = 0;
}

template <typename T, uint16_t qsize>
T * Queue<T, qsize>::data()
{
	return array;
}

template <typename T, uint16_t qsize>
uint8_t Queue<T, qsize>::push_back(T const &item)
{
	// If storage has no space return.
	if (full) return 0;

	// When storage has right side free space.
	if (end_index >= start_index) {
		if (end_index < qsize)
		{
			array[end_index++] = item;
		}
		// Storage has left empty space.
		else if (start_index > 0)
		{
			// Store data precending the start_index pointer.
			end_index = 0;
			array[end_index++] = item;
		}
	}
	else
	{
		array[end_index++] = item;
	}

	// Check for full state.
	if (end_index >= qsize || end_index == start_index)
	{
		full = QUEUE_FULL;
	}

	return 1;
}

template <typename T, uint16_t qsize>
T & Queue<T, qsize>::front()
{
	return array[start_index];
}

template <typename T, uint16_t qsize>
T & Queue<T, qsize>::back()
{
	return array[end_index - 1];
}

template <typename T, uint16_t qsize>
void Queue<T, qsize>::pop_front()
{
	if (this->empty()) return;

	if (start_index < qsize) ++start_index;

	if (this->empty())
	{
		// return start_index\end_index pointers to initial positions.
		this->clear();

		return;
	}

	if (start_index >= qsize)
	{
		start_index = 0;
	}

	if (full) full = QUEUE_IS_NOT_FULL;
}
