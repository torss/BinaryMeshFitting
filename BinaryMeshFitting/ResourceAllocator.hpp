#pragma once

#include <mutex>
#include "MemoryPool.h"
#include "LinkedList.hpp"

template <class T>
class ResourceAllocator
{
public:
	inline ResourceAllocator()
	{
	}

	inline ~ResourceAllocator()
	{
		LinkedNode<T>* n = used_chunks.head;
		while (n)
		{
			LinkedNode<T>* next = n->next;
			pool.deleteElement((T*)n);
			n = next;
		}

		n = free_chunks.head;
		while (n)
		{
			LinkedNode<T>* next = n->next;
			pool.deleteElement((T*)n);
			n = next;
		}
	}

	inline T* new_element(bool no_lock = false)
	{
		std::unique_lock<std::mutex> _lock(_mutex, std::defer_lock);
		if (!no_lock)
			_lock.lock();
		if (free_chunks.tail)
		{
			T* c = (T*)free_chunks.tail;
			free_chunks.unlink(c);
			used_chunks.push_back(c);
			return c;
		}

		T* c = pool.newElement();
		used_chunks.push_back(c);
		return c;
	}

	inline void free_element(T* element, bool no_lock = false)
	{
		if (!element)
			return;
		std::unique_lock<std::mutex> lock(_mutex, std::defer_lock);
		if (!no_lock)
			lock.lock();
		free_chunks.push_back(used_chunks.unlink(element));
		if (!no_lock)
			lock.unlock();
	}

	std::mutex _mutex;

private:
	MemoryPool<T> pool;
	LinkedList<T> used_chunks;
	LinkedList<T> free_chunks;
};