#pragma once

#include "type.h"

namespace qbind
{

template <Type>
class Atom;

template <Type>
class Vector;

template <Type>
class NestedVector;

template <class... Types>
class Tuple;

template <class TKey, class TValue>
class Dictionary;

class Converter;

// Abbreviation: Q style and C style

namespace a
{
using b = qbind::Atom<Type::Boolean>;
using g = qbind::Atom<Type::GUID>;
using x = qbind::Atom<Type::Byte>;
using h = qbind::Atom<Type::Short>;
using i = qbind::Atom<Type::Int>;
using j = qbind::Atom<Type::Long>;
using e = qbind::Atom<Type::Real>;
using f = qbind::Atom<Type::Float>;
using c = qbind::Atom<Type::Char>;   
using s = qbind::Atom<Type::Symbol>;
using p = qbind::Atom<Type::Timestamp>; 
using m = qbind::Atom<Type::Month>; 
using d = qbind::Atom<Type::Date>;
using z = qbind::Atom<Type::Datetime>; 
using n = qbind::Atom<Type::Timespan>;
using u = qbind::Atom<Type::Minute>;
using v = qbind::Atom<Type::Second>;  
using t = qbind::Atom<Type::Time>;   
} // namespace a

namespace atom
{
using Boolean   = qbind::Atom<Type::Boolean>;
using GUID      = qbind::Atom<Type::GUID>;
using Byte      = qbind::Atom<Type::Byte>;
using Short     = qbind::Atom<Type::Short>;
using Int       = qbind::Atom<Type::Int>;
using Long      = qbind::Atom<Type::Long>;
using Real      = qbind::Atom<Type::Real>;
using Float     = qbind::Atom<Type::Float>;
using Char      = qbind::Atom<Type::Char>;   
using Symbol    = qbind::Atom<Type::Symbol>;
using Timestamp = qbind::Atom<Type::Timestamp>; 
using Month     = qbind::Atom<Type::Month>; 
using Date      = qbind::Atom<Type::Date>;
using Datetime  = qbind::Atom<Type::Datetime>; 
using Timespan  = qbind::Atom<Type::Timespan>;
using Minute    = qbind::Atom<Type::Minute>;
using Second    = qbind::Atom<Type::Second>;  
using Time      = qbind::Atom<Type::Time>; 
} // namespace atom

namespace v
{
using b = qbind::Vector<Type::Boolean>;
using g = qbind::Vector<Type::GUID>;
using x = qbind::Vector<Type::Byte>;
using h = qbind::Vector<Type::Short>;
using i = qbind::Vector<Type::Int>;
using j = qbind::Vector<Type::Long>;
using e = qbind::Vector<Type::Real>;
using f = qbind::Vector<Type::Float>;
using c = qbind::Vector<Type::Char>;   
using s = qbind::Vector<Type::Symbol>;
using p = qbind::Vector<Type::Timestamp>; 
using m = qbind::Vector<Type::Month>; 
using d = qbind::Vector<Type::Date>;
using z = qbind::Vector<Type::Datetime>; 
using n = qbind::Vector<Type::Timespan>;
using u = qbind::Vector<Type::Minute>;
using v = qbind::Vector<Type::Second>;  
using t = qbind::Vector<Type::Time>;   
} // namespace v

namespace vector
{
using Boolean   = qbind::Vector<Type::Boolean>;
using GUID      = qbind::Vector<Type::GUID>;
using Byte      = qbind::Vector<Type::Byte>;
using Short     = qbind::Vector<Type::Short>;
using Int       = qbind::Vector<Type::Int>;
using Long      = qbind::Vector<Type::Long>;
using Real      = qbind::Vector<Type::Real>;
using Float     = qbind::Vector<Type::Float>;
using Char      = qbind::Vector<Type::Char>;   
using Symbol    = qbind::Vector<Type::Symbol>;
using Timestamp = qbind::Vector<Type::Timestamp>; 
using Month     = qbind::Vector<Type::Month>; 
using Date      = qbind::Vector<Type::Date>;
using Datetime  = qbind::Vector<Type::Datetime>; 
using Timespan  = qbind::Vector<Type::Timespan>;
using Minute    = qbind::Vector<Type::Minute>;
using Second    = qbind::Vector<Type::Second>;  
using Time      = qbind::Vector<Type::Time>; 
} // namespace vector

using String = qbind::Vector<Type::Char>;

namespace n
{
using b = qbind::NestedVector<Type::Boolean>;
using g = qbind::NestedVector<Type::GUID>;
using x = qbind::NestedVector<Type::Byte>;
using h = qbind::NestedVector<Type::Short>;
using i = qbind::NestedVector<Type::Int>;
using j = qbind::NestedVector<Type::Long>;
using e = qbind::NestedVector<Type::Real>;
using f = qbind::NestedVector<Type::Float>;
using c = qbind::NestedVector<Type::Char>;   
using s = qbind::NestedVector<Type::Symbol>;
using p = qbind::NestedVector<Type::Timestamp>; 
using m = qbind::NestedVector<Type::Month>; 
using d = qbind::NestedVector<Type::Date>;
using z = qbind::NestedVector<Type::Datetime>; 
using n = qbind::NestedVector<Type::Timespan>;
using u = qbind::NestedVector<Type::Minute>;
using v = qbind::NestedVector<Type::Second>;  
using t = qbind::NestedVector<Type::Time>;   
} // namespace n

namespace nested_vector
{
using Boolean   = qbind::NestedVector<Type::Boolean>;
using GUID      = qbind::NestedVector<Type::GUID>;
using Byte      = qbind::NestedVector<Type::Byte>;
using Short     = qbind::NestedVector<Type::Short>;
using Int       = qbind::NestedVector<Type::Int>;
using Long      = qbind::NestedVector<Type::Long>;
using Real      = qbind::NestedVector<Type::Real>;
using Float     = qbind::NestedVector<Type::Float>;
using Char      = qbind::NestedVector<Type::Char>;   
using Symbol    = qbind::NestedVector<Type::Symbol>;
using Timestamp = qbind::NestedVector<Type::Timestamp>; 
using Month     = qbind::NestedVector<Type::Month>; 
using Date      = qbind::NestedVector<Type::Date>;
using Datetime  = qbind::NestedVector<Type::Datetime>; 
using Timespan  = qbind::NestedVector<Type::Timespan>;
using Minute    = qbind::NestedVector<Type::Minute>;
using Second    = qbind::NestedVector<Type::Second>;  
using Time      = qbind::NestedVector<Type::Time>; 
} // namespace vector

}