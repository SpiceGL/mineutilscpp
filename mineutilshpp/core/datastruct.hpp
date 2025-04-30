#pragma once
#include<type_traits>
#include<vector>

#include"base.hpp"
#include"type.hpp"



namespace mineutils
{
	/*--------------------------------------------用户接口--------------------------------------------*/

	namespace mds
	{
		/*	循环队列
			- T不能有const、volatile或引用符号修饰
			- T不可为void
			- T必须存在无参构造函数
			- T必须能够由自身的左值或右值赋值  */
		template<class T>
		class CircularQueue
		{
		public:
			//构造一个容量为0的空对象
			template<class U = T, typename std::enable_if<std::is_same<U, T>::value&& mtype::_mpriv::CircularQueueElemChecker<U>::value, int>::type = 0>
			CircularQueue() {};

			/*	构造循环队列；模板参数仅用于类型检查，不可修改
				@param queue_capacity: 循环队列的容量，为0时入队出队都无效  */
			template<class U = T, typename std::enable_if<std::is_same<U, T>::value&& mtype::_mpriv::CircularQueueElemChecker<U>::value, int>::type = 0>
			CircularQueue(size_t queue_capacity);

			/*	作为左值入队
				- T能够由自身的左值赋值时，接口实例化
				- 模板参数仅用于类型检查，不可修改
				@param value: 要入队的元素  */
			template<class U = T, typename std::enable_if<std::is_same<U, T>::value&& std::is_assignable<U&, const U&>::value, int>::type = 0>
			void enqueue(const T& value);

			/*	作为右值入队
				- T能够由自身的右值赋值时，接口实例化
				- 模板参数仅用于类型检查，不可修改
				@param value: 要入队的元素  */
			template<class U = T, typename std::enable_if<std::is_same<U, T>::value&& std::is_assignable<U&, U&&>::value, int>::type = 0>
			void enqueue(T&& value);

			/*	尝试出队，队列为空时出队失败
				@param dst: 用于接收出队的元素，出队成功时将被赋值
				@return 成功时返回true，队列为空时返回false  */
			bool tryDequeue(T& dst);

			//队列是否为空
			bool empty();
			//队列是否已满
			bool full();

			//支持移动，禁止拷贝
			CircularQueue(CircularQueue<T>&& rvalue) noexcept;
			CircularQueue<T>& operator=(CircularQueue<T>&& rvalue) noexcept;

		private:
			void move(CircularQueue<T>&& rvalue);
			template<class U, typename std::enable_if<std::is_assignable<U&, U&&>::value, int>::type = 0>
			bool tryDequeueDispatch(U& dst);
			template<class U, typename std::enable_if<!std::is_assignable<U&, U&&>::value, int>::type = 0>
			bool tryDequeueDispatch(U& dst);

			size_t cache_num_ = 0;
			std::vector<T> buffer_;
			size_t head_ = 0;
			size_t tail_ = 0;
			size_t count_ = 0;
		};



	}



















	/*--------------------------------------------内部实现--------------------------------------------*/

	namespace mds
	{
		template<class T>
		template<class U, typename std::enable_if<std::is_same<U, T>::value&& mtype::_mpriv::CircularQueueElemChecker<U>::value, int>::type>
		inline CircularQueue<T>::CircularQueue(size_t queue_capacity)
		{
			this->cache_num_ = queue_capacity;
			this->buffer_.resize(queue_capacity);
			this->head_ = 0;
			this->tail_ = 0;
			this->count_ = 0;
		}

		template<class T>
		template<class U, typename std::enable_if<std::is_same<U, T>::value&& std::is_assignable<U&, const U&>::value, int>::type>
		inline void CircularQueue<T>::enqueue(const T& value)
		{
			if (this->buffer_.empty())
			{
				mprintfW("Capacity is 0!\n");
				return;
			}

			if (this->count_ == this->cache_num_)
				this->head_ = (this->head_ + 1) % this->cache_num_;
			else this->count_++;

			this->buffer_[this->tail_] = value;
			this->tail_ = (this->tail_ + 1) % this->cache_num_;
		}

		template<class T>
		template<class U, typename std::enable_if<std::is_same<U, T>::value&& std::is_assignable<U&, U&&>::value, int>::type>
		inline void CircularQueue<T>::enqueue(T&& value)
		{
			if (this->buffer_.empty())
			{
				mprintfW("Capacity is 0!\n");
				return;
			}
			if (this->count_ == this->cache_num_)
				this->head_ = (this->head_ + 1) % this->cache_num_;
			else this->count_++;

			this->buffer_[this->tail_] = std::move(value);
			this->tail_ = (this->tail_ + 1) % this->cache_num_;
		}

		template<class T>
		inline bool CircularQueue<T>::tryDequeue(T& dst)
		{
			if (this->buffer_.empty())
			{
				mprintfW("Capacity is 0!\n");
				return false;
			}
			return this->tryDequeueDispatch(dst);
		}

		template<class T>
		inline bool CircularQueue<T>::empty()
		{
			return this->count_ == 0;
		}

		template<class T>
		inline bool CircularQueue<T>::full()
		{
			return this->count_ == this->cache_num_;
		}

		template<class T>
		inline CircularQueue<T>::CircularQueue(CircularQueue<T>&& rvalue) noexcept
		{
			this->move(std::move(rvalue));
		}
		template<class T>
		inline CircularQueue<T>& CircularQueue<T>::operator=(CircularQueue<T>&& rvalue) noexcept
		{
			if (this != &rvalue)
				this->move(std::move(rvalue));
			return *this;
		}

		template<class T>
		inline void CircularQueue<T>::move(CircularQueue<T>&& rvalue)
		{
			this->buffer_ = std::move(rvalue.buffer_);
			this->cache_num_ = rvalue.cache_num_;
			this->head_ = rvalue.head_;
			this->tail_ = rvalue.tail_;
			this->count_ = rvalue.count_;

			rvalue.cache_num_ = 0;
			rvalue.head_ = 0;
			rvalue.tail_ = 0;
			rvalue.count_ = 0;
		}

		template<class T>
		template<class U, typename std::enable_if<std::is_assignable<U&, U&&>::value, int>::type>
		inline bool CircularQueue<T>::tryDequeueDispatch(U& dst)
		{
			if (this->empty())
				return false;
			dst = std::move(this->buffer_[this->head_]);
			this->head_ = (this->head_ + 1) % this->cache_num_;
			this->count_--;
			return true;
		}

		template<class T>
		template<class U, typename std::enable_if<!std::is_assignable<U&, U&&>::value, int>::type>
		inline bool CircularQueue<T>::tryDequeueDispatch(U& dst)
		{
			if (this->empty())
				return false;
			dst = this->buffer_[this->head_];
			this->head_ = (this->head_ + 1) % this->cache_num_;
			this->count_--;
			return true;
		}
	}







#ifdef MINEUTILS_TEST_MODULES
	namespace _mdscheck
	{
		class NoMove
		{
		public:
			NoMove() {};
			NoMove(const NoMove& obj) = default;
			NoMove(NoMove&& obj) = delete;
			NoMove& operator=(const NoMove& obj) = default;
			NoMove& operator=(NoMove&& obj) = delete;
		};
		inline void CircularQueueTest()
		{
			mds::CircularQueue<int> queue{ 3 };
			queue.enqueue(1);
			queue.enqueue(2);
			queue.enqueue(3);
			queue.enqueue(4);
			int dst = 0;

			int ret0 = (queue.tryDequeue(dst), dst == 2);
			if (!ret0) mprintfE(R"(Failed when check: queue.tryDequeue(dst), dst == 2)""\n");

			NoMove no_move;
			mds::CircularQueue<NoMove> queue1{ 3 };
			mds::CircularQueue<NoMove> queue2 = std::move(queue1);

			queue1.enqueue(std::move(no_move));
		}

		inline void check()
		{
			printf("\n--------------------check mds start--------------------\n");
			CircularQueueTest();
			printf("---------------------check mds end---------------------\n\n");
		}
	}
#endif
}