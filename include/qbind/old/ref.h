#pragma once

#include <string>
#include <functional>

namespace qbind
{

template<typename Adapter>
class Ref
{
public:
    Ref(typename Adapter::Underlier& v)
        :val_(v)
    {}

    using Underlier = typename Adapter::Underlier;
    using Target = typename Adapter::Target;

    /**
     * @brief other is target move assigned to this
     */
    Ref<Adapter>& operator=(Target&& other)
    {
        adapter_.setValue(val_, std::move(other));
        return *this;
    }

    /**
     * @brief other is target which is copy assigned to this
     */
    Ref<Adapter>& operator=(const Target& other)
    {
        adapter_.setValue(val_, other);
        return *this;
    }

    // TODO: replace with proper operators to not expose raw ref
    Underlier &operator*() { return val_; }

    /**
     * @brief convert this reference implicitly to the target type.
     */
    operator Target() const { return adapter_.getValue(val_); }

    template<typename Element>
    friend std::ostream& operator<<(std::ostream& os, const Ref<Element>& ref);

private:
    Adapter adapter_;
    Underlier &val_;
};

template<typename Element>
std::ostream& operator<<(std::ostream& os, const Ref<Element>& ref)
{
    os << ref.val_;
    return os;
}

template<typename Adapter>
class ConstRef
{
public:
    ConstRef(typename Adapter::Underlier& v)
        :val_(v)
    {}

    using Underlier = typename Adapter::Underlier;
    using Target = typename Adapter::Target;

    constexpr ConstRef<Adapter>& operator=(Target other) const
    {
        constexpr auto false_ = [](){ return false; };
        static_assert(false_(), "Cannot assign to a const ref.");
    }

    // TODO: provide proper operators on the underlying

    operator Target() const { return adapter_.getValue(val_); }

    template<typename Element>
    friend std::ostream& operator<<(std::ostream& os, const ConstRef<Element>& ref);

private:
    Adapter adapter_;
    Underlier &val_;
};

template<typename Element>
std::ostream& operator<<(std::ostream& os, const ConstRef<Element>& ref)
{
    os << ref.val_;
    return os;
}

}