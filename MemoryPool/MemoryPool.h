#pragma once

#include <cstdlib>
#include <iostream>
using namespace std;


template<class T>
class ObjectPool
{
	struct BlockNode
	{
		void* _memory;		//ָ���ڴ���ָ��
		BlockNode* _next;	//ָ����һ������ָ��
		size_t _objNum;		//��ʾ�ڴ����ĸ���

		BlockNode(size_t objNum)
			:_next(NULL)
			,_objNum(objNum)
		{
			_memory = malloc(_itemSize * _objNum);
		}

		~BlockNode()
		{
			free(_memory);	//���ͷŽ�����ҵ��ڴ��
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
		//1.����ʹ����ǰ�ͷŵ��ڴ�
		//2.���ڴ����������
		//3.�����µĽ�����

		if (_lastDelete)	//�������ͷŻ������ڴ��ʱ����ʹ����Щ�ڴ��
		{
			T* obj = _lastDelete;	//�¿��ٿռ���Ƚ�_lastDelete��ָ��ռ�ĵ�ַ��������
			_lastDelete = *((T**)_lastDelete);	//�õ��¿��ٵ�һ���ڴ�ĵ�ַ
			return new(obj)T;	//�Ѵ�����ڴ�����һ��T�ֽڵ��ڴ���г�ʼ��
		}

		if (_countIn >= _last->_objNum)	//��ʾ��ʱ�ýڵ�����ָ����ڴ��Ѿ����꣬��Ҫ���¿���һ�����
		{
			AllocateNewNode();
			//size_t newsize = _countIn * 2;
			//if (newsize > _maxNum)//����������ڴ��С�������ֵ�ͽ����ֵ��Ϊnewsize
			//	newsize = _maxNum;

			//_last->_next = new BlockNode(newsize);
			//_last = _last->_next;
			//_countIn = 0;
		}
		T* obj = (T*)((char*)_last->_memory + _countIn*_itemSize);
		_countIn++;
		return new(obj)T;
	}

	void Delete(T* ptr)	//�ͷ��ڴ��
	{
		ptr->~T();
		if (ptr)
		{
			*((T**)ptr) = _lastDelete;	//�ȱ�����һ���ͷŵĿռ�ĵ�ַ
			_lastDelete = ptr;		//��_lastDelete�Ƶ����һ���ͷŵĿռ�ptr��
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
			delete del;	//��ɾ�����ʱ����~BlockNode()
		}
	}
	static size_t InitemSize()
	{
		//T�����п��ܱ�һ��ָ��С�����������ǵ�ַ��������Ҫһ��ָ��Ĵ�С
		if(sizeof(T) > sizeof(void*))
			return sizeof(T);
		else
			return sizeof(void*);
	}
protected:
	size_t _countIn;	//ͳ�Ƶ�ǰ������õļ���
	BlockNode* _first;	//ָ���������ͷָ��
	BlockNode* _last;	//βָ��
	size_t _maxNum;		//������ڴ�������С
	T* _lastDelete;		//ָ�����һ���ͷŵĶ���
	static size_t _itemSize;//��������Ĵ�С
};

//�����ʼ����̬����
template<class T>
size_t ObjectPool<T>::_itemSize = ObjectPool<T>::InitemSize();