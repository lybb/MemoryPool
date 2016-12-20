#pragma once

#include <cstdlib>
#include <iostream>
using namespace std;


template<class T>
class ObjectPool
{
	struct BlockNode
	{
		void* _memory;		//指向内存块的指针
		BlockNode* _next;	//指向下一个结点的指针
		size_t _objNum;		//表示内存对象的个数

		BlockNode(size_t objNum)
			:_next(NULL)
			,_objNum(objNum)
		{
			_memory = malloc(_itemSize * _objNum);
		}

		~BlockNode()
		{
			free(_memory);	//先释放结点所挂的内存块
			_memory = NULL;
			_next = NULL;
		}
	};
public:
	ObjectPool(size_t initnum = 32,size_t maxNum = 10000)
		:_countIn(0)
		,_maxNum(maxNum)
		,_lastDelete(NULL)
	{
		_first = _last = new BlockNode(initnum);
	}
	~ObjectPool()
	{
		Destory();
		_first = _last = NULL;
	}

	T* New()
	{
		//1.优先使用以前释放的内存
		//2.在内存块里面申请
		//3.申请新的结点对象

		if (_lastDelete)	//当还有释放回来的内存块时优先使用这些内存块
		{
			T* obj = _lastDelete;	//新开辟空间后先将_lastDelete所指向空间的地址保存下来
			_lastDelete = *((T**)_lastDelete);	//得到新开辟的一块内存的地址
			return new(obj)T;	//把从这块内存往后，一个T字节的内存进行初始化
		}

		if (_countIn >= _last->_objNum)	//表示此时该节点下所指向的内存已经用完，需要重新开辟一个结点
		{
			AllocateNewNode();
			//size_t newsize = _countIn * 2;
			//if (newsize > _maxNum)//如果新增的内存大小大于最大值就将最大值作为newsize
			//	newsize = _maxNum;

			//_last->_next = new BlockNode(newsize);
			//_last = _last->_next;
			//_countIn = 0;
		}
		T* obj = (T*)((char*)_last->_memory + _countIn*_itemSize);
		_countIn++;
		return new(obj)T;
	}

	void Delete(T* ptr)	//释放内存块
	{
		ptr->~T();
		if (ptr)
		{
			*((T**)ptr) = _lastDelete;	//先保存上一个释放的空间的地址
			_lastDelete = ptr;		//将_lastDelete移到最近一次释放的空间ptr上
		}		
	}

protected:
	void AllocateNewNode()
	{
		size_t newsize = 2*_countIn;
		if (newsize > _maxNum)	
			newsize = _maxNum;

		_last->_next = new BlockNode(newsize);
		_last = _last->_next;
		_countIn = 0;
	}
	void Destory()
	{
		BlockNode* cur = _first;
		BlockNode* del = NULL;
		while (cur)
		{
			del = cur;
			cur = cur->_next;
			delete del;	//当删除结点时调用~BlockNode()
		}
	}
	static size_t InitemSize()
	{
		//T类型有可能比一个指针小，但这里存的是地址，最少需要一个指针的大小
		if(sizeof(T) > sizeof(void*))
			return sizeof(T);
		else
			return sizeof(void*);
	}
protected:
	size_t _countIn;	//统计当前结点在用的计数
	BlockNode* _first;	//指向链表结点的头指针
	BlockNode* _last;	//尾指针
	size_t _maxNum;		//申请的内存块的最大大小
	T* _lastDelete;		//指向最后一个释放的对象
	static size_t _itemSize;//单个对象的大小
};

//类外初始化静态变量
template<class T>
size_t ObjectPool<T>::_itemSize = ObjectPool<T>::InitemSize();