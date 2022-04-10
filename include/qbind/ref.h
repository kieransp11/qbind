#pragma once

#include <string>
#include <functional>

namespace qbind
{

template<typename Type>
class Ref
{
public:
    Ref(typename Type::Underlier& v)
        :val_(v)
    {}

    using Underlier = typename Type::Underlier;
    using Target = typename Type::Target;

    /**
     * @brief other is target move assigned to this
     */
    Ref<Type>& operator=(Target&& other)
    {
        type_.setValue(val_, std::move(other));
        return *this;
    }

    /**
     * @brief other is target which is copy assigned to this
     */
    Ref<Type>& operator=(const Target& other)
    {
        type_.setValue(val_, other);
        return *this;
    }

    // TODO: replace with proper operators to not expose raw ref
    Underlier &operator*() { return val_; }

    /**
     * @brief convert this reference implicitly to the target type.
     */
    operator Target() const { return type_.getValue(val_); }

    template<typename Element>
    friend std::ostream& operator<<(std::ostream& os, const Ref<Element>& ref);

private:
    Type type_;
    Underlier &val_;
};

template<typename Element>
std::ostream& operator<<(std::ostream& os, const Ref<Element>& ref)
{
    os << ref.val_;
    return os;
}

template<typename Type>
class ConstRef
{
public:
    using ElementType = Type;

    ConstRef(typename Type::Underlier& v)
        :val_(v)
    {}

    using Underlier = typename Type::Underlier;
    using Target = typename Type::Target;

    constexpr ConstRef<Type>& operator=(Target other) const
    {
        constexpr auto false_ = [](){ return false; };
        static_assert(false_(), "Cannot assign to a const ref.");
    }

    // TODO: provide proper operators on the underlying

    operator Target() const { return type_.getValue(val_); }

    template<typename Element>
    friend std::ostream& operator<<(std::ostream& os, const ConstRef<Element>& ref);

private:
    Type type_;
    Underlier &val_;
};

template<typename Element>
std::ostream& operator<<(std::ostream& os, const ConstRef<Element>& ref)
{
    os << ref.val_;
    return os;
}

}