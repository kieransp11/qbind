#pragma once

#include "type.h"

namespace qbind
{

/**
 * @brief Visit function with arguments.This only visits basic types.
 * 
 * If you need to be able to determine if an atom or array code was passed,
 * use Visit(K, Fn, Args&&) instead.
 * 
 * @tparam Fn : function to apply with arguments (Type<size_>, Args...);
 * @tparam Args : additional arguments to the function
 * @param code : runtime type code of a kdb K object
 * @param function : function to apply to (Type<size_t>, Args..);
 * @param args : additional arguments to function.
 * @return auto 
 */
template<typename Fn, typename... Args>
auto Visit(Fn function, int8_t code, Args&&... args)
{
    switch (std::abs(code))
    {
        case 0:     return function(Q::Mixed(), std::forward<Args>(args)...);
        case KB:    return function(Q::Boolean(), std::forward<Args>(args)...);
        case UU:    return function(Q::Guid(), std::forward<Args>(args)...);
        case KG:    return function(Q::Byte(), std::forward<Args>(args)...);
        case KH:    return function(Q::Short(), std::forward<Args>(args)...);
        case KI:    return function(Q::Integer(), std::forward<Args>(args)...);
        case KJ:    return function(Q::Long(), std::forward<Args>(args)...);
        case KE:    return function(Q::Real(), std::forward<Args>(args)...);
        case KF:    return function(Q::Float(), std::forward<Args>(args)...);
        case KC:    return function(Q::Char(), std::forward<Args>(args)...);
        case KS:    return function(Q::Symbol(), std::forward<Args>(args)...);
        case KP:    return function(Q::Timestamp(), std::forward<Args>(args)...);
        case KM:    return function(Q::Month(), std::forward<Args>(args)...);
        case KD:    return function(Q::Date(), std::forward<Args>(args)...);
        case KN:    return function(Q::Timespan(), std::forward<Args>(args)...);
        case KU:    return function(Q::Minute(), std::forward<Args>(args)...);
        case KV:    return function(Q::Second(), std::forward<Args>(args)...);
        case KT:    return function(Q::Time(), std::forward<Args>(args)...);
        case KZ:    return function(Q::Datetime(), std::forward<Args>(args)...);
        case 128:   return function(Q::Error(), std::forward<Args>(args)...);
    }
    std::ostringstream s;
    s << "Attempted to visit on unknown KDB type code " << std::to_string(code) << ".";
    throw std::invalid_argument(s.str());
}

/**
 * @brief Visitor on runtime type code of karr. Passes karr and args to function.
 */
template<typename Fn, typename... Args>
auto Visit(Fn function, K karr, Args&&... args)
{
    if (karr == nullptr)
        throw std::invalid_argument("Cannot visit a null K object");
    return Visit(function, karr->t, karr, std::forward<Args>(args)...);
}

// std::string to_name(int8_t code)
// {
//     auto name = Visit([](const auto& t){ return t.name; }, code);
//     std::string suffix = code < 0 ? " Atom" : " List";
//     return name + suffix;
// }

}