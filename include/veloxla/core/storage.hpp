#pragma once
// VeloxLA — core/storage.hpp — contiguous memory storage, ownership

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

namespace veloxla
{

template<typename T, typename Allocator = std::allocator<T>>
class Storage
{
public:

    using value_type      = T;
    using allocator_type  = Allocator;
    using size_type       = std::size_t;

    using reference       = T&;
    using const_reference = const T&;

    using pointer         = T*;
    using const_pointer   = const T*;

private:

    allocator_type allocator_;
    pointer data_;
    size_type size_;

private:

    pointer allocate(size_type n)
    {
        if(n == 0)
        {
            return nullptr;
        }

        return std::allocator_traits<allocator_type>::allocate(
            allocator_,
            n
        );
    }

    void deallocate() noexcept
    {
        if(data_ != nullptr)
        {
            std::allocator_traits<allocator_type>::deallocate(
                allocator_,
                data_,
                size_
            );

            data_ = nullptr;
        }
    }

    void construct_all()
    {
        size_type i = 0;

        try
        {
            for(; i < size_; ++i)
            {
                std::allocator_traits<allocator_type>::construct(
                    allocator_,
                    data_ + i
                );
            }
        }
        catch(...)
        {
            for(size_type j = i; j-- > 0;)
            {
                std::allocator_traits<allocator_type>::destroy(
                    allocator_,
                    data_ + j
                );
            }

            deallocate();
            throw;
        }
    }

    // Mengasumsikan class invariant (lihat komentar di atas class): jika
    // data_ != nullptr, seluruh [0, size_) sudah constructed.
    void destroy_all() noexcept
    {
        for(size_type i = size_; i-- > 0;)
        {
            std::allocator_traits<allocator_type>::destroy(
                allocator_,
                data_ + i
            );
        }
    }

public:

    Storage() noexcept
        : allocator_()
        , data_(nullptr)
        , size_(0)
    {
    }

    explicit Storage(size_type size)
        : allocator_()
        , data_(allocate(size))
        , size_(size)
    {
        construct_all();
    }

    Storage(
        size_type size,
        const allocator_type& allocator
    )
        : allocator_(allocator)
        , data_(allocate(size))
        , size_(size)
    {
        construct_all();
    }

    Storage(
        size_type size,
        const_reference fill_value
    )
        : allocator_()
        , data_(allocate(size))
        , size_(size)
    {
        size_type i = 0;

        try
        {
            for(; i < size_; ++i)
            {
                std::allocator_traits<allocator_type>::construct(
                    allocator_,
                    data_ + i,
                    fill_value
                );
            }
        }
        catch(...)
        {
            for(size_type j = i; j-- > 0;)
            {
                std::allocator_traits<allocator_type>::destroy(
                    allocator_,
                    data_ + j
                );
            }

            deallocate();
            throw;
        }
    }

    ~Storage() noexcept
    {
        destroy_all();
        deallocate();
    }

    Storage(const Storage&) = delete;
    Storage& operator=(const Storage&) = delete;

    Storage(Storage&& other) noexcept
        : allocator_(std::move(other.allocator_))
        , data_(other.data_)
        , size_(other.size_)
    {
        other.data_ = nullptr;
        other.size_ = 0;
    }

    // Sengaja TIDAK ditulis sebagai "swap(other); return *this;" meski itu
    // lebih singkat. Versi swap menunda pelepasan buffer lama *this sampai
    // 'other' (yang sekarang memegangnya) keluar scope — untuk temporary
    // hasil ekspresi, itu artinya sampai akhir full-expression. Untuk
    // tensor besar, menunda pelepasan memori seperti itu kurang predictable
    // dibanding melepas buffer lama secara eksplisit di awal seperti di
    // bawah ini.
    Storage& operator=(Storage&& other) noexcept
    {
        if(this == &other)
        {
            return *this;
        }

        destroy_all();
        deallocate();

        allocator_ = std::move(other.allocator_);
        data_      = other.data_;
        size_      = other.size_;

        other.data_ = nullptr;
        other.size_ = 0;

        return *this;
    }

    void swap(Storage& other) noexcept
    {
        using std::swap;

        swap(allocator_, other.allocator_);
        swap(data_, other.data_);
        swap(size_, other.size_);
    }

    [[nodiscard]]
    size_type size() const noexcept
    {
        return size_;
    }

    [[nodiscard]]
    bool empty() const noexcept
    {
        return size_ == 0;
    }

    reference operator[](size_type index) noexcept
    {
        return data_[index];
    }

    const_reference operator[](size_type index) const noexcept
    {
        return data_[index];
    }

    reference at(size_type index)
    {
        if(index >= size_)
        {
            throw std::out_of_range(
                "Storage::at - index out of range"
            );
        }

        return data_[index];
    }

    const_reference at(size_type index) const
    {
        if(index >= size_)
        {
            throw std::out_of_range(
                "Storage::at - index out of range"
            );
        }

        return data_[index];
    }

    pointer data() noexcept
    {
        return data_;
    }

    const_pointer data() const noexcept
    {
        return data_;
    }

    pointer begin() noexcept
    {
        return data_;
    }

    const_pointer begin() const noexcept
    {
        return data_;
    }

    const_pointer cbegin() const noexcept
    {
        return data_;
    }

    pointer end() noexcept
    {
        return data_ + size_;
    }

    const_pointer end() const noexcept
    {
        return data_ + size_;
    }

    const_pointer cend() const noexcept
    {
        return data_ + size_;
    }

    // Convenience untuk tensor numerik — setara std::fill(begin(), end(),
    // value) tapi dipanggil langsung dari Storage tanpa perlu include
    // <algorithm> di call site.
    void fill(const_reference value)
    {
        std::fill(begin(), end(), value);
    }
};

} // namespace veloxla