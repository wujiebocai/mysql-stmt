#pragma once

#include <new>
#include <iostream>
#include <cstdlib> 
#include <climits> 
#include <cstddef>

#include <util/bytebuffer.hpp>

namespace mysql_detail {
	constexpr static int use_buff_maxsize = 128; //小于128的对象使用缓存
	template<size_t SizeN>
	class alloc_memory {
	public:
		explicit alloc_memory() {}

		alloc_memory(const alloc_memory& allm) = delete;
		alloc_memory& operator=(const alloc_memory&) = delete;

		inline void* _allocate(size_t size) {
			void* addr = nullptr;
			auto buffsize = buff_.wr_size();
			if (size < use_buff_maxsize && size <= buffsize) {
				addr = (void*)buff_.wr_buf();
				buff_.wr_flip(size);
			}
			else {
				addr = ::operator new(size);
			}
			if (!addr) {
				std::cerr << "out of memory" << std::endl;
			}
			return addr;
		}

		inline void _deallocate(void* pointer) {
			if (buff_.is_range(pointer)) {
				//缓存管理后面优化,暂时只能分配,不能重新利用缓存.
			}
			else {
				::operator delete[](pointer);
			}
		}

		template<class T>
		inline T* _allocate(ptrdiff_t size, T*) {
			std::set_new_handler(0);
			size_t rsize = size * sizeof(T);
			return (static_cast<T *>(_allocate(rsize)));
		}

		template<class T>
		inline void _deallocate(T* p, size_t n) {
			_deallocate(p);
		}

	private:
		t_buffer_cmdqueue<> buff_;
	};

	template<class T>
	class allocator_m {
	public:
		using _Not_user_specialized = void;
		using value_type = T;
		using propagate_on_container_move_assignment = std::true_type;
		using is_always_equal = std::true_type;

		typedef T*              pointer;
		typedef const T*        const_pointer;
		typedef T&              reference;
		typedef const T&        const_reference;
		typedef size_t          size_type;
		typedef ptrdiff_t       difference_type;

		template<class U>
		struct rebind {
			typedef allocator_m<U> other;
		};

		constexpr allocator_m() noexcept {
		}
		constexpr allocator_m(const allocator_m&) noexcept = default;
		template<class _Other>
		constexpr allocator_m(const allocator_m<_Other>& alloc) : mem_(alloc.mem_) {
		}

		T * allocate(const size_t n) {
			return mem_->_allocate((difference_type)n, (pointer)0);
		}

		pointer allocate(const size_t n, const void* hint) {
			return allocate(n);
		}

		void deallocate(T * const ptr, const size_t n) {
			return mem_->_deallocate(ptr, n);
		}

		template<class objtype, class... Args>
		void construct(objtype * const ptr, Args&&... _Args) {
			::new (const_cast<void *>(static_cast<const volatile void *>(ptr)))
				objtype(std::forward<Args>(_Args)...);
		}

		template<class objtype>
		void destroy(objtype * const ptr) {
			ptr->~objtype();
		}

		T * address(T& _Val) const noexcept {
			return (std::addressof(_Val));
		}
		const T * address(const T& _Val) const noexcept {
			return (std::addressof(_Val));
		}

		size_t max_size() const noexcept {
			return (static_cast<size_t>(-1) / sizeof(T));
		}

	//private:
		std::shared_ptr<alloc_memory<1024>> mem_ = std::make_shared<alloc_memory<1024>>();
	};

	template<>
	class allocator_m<void> {
	public:
		using value_type = void;
		using pointer = void *;
		using const_pointer = const void *;

		template<class _Other>
		struct rebind {
			using other = allocator_m<_Other>;
		};
	};
}
