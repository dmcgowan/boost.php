//
// Copyright 2008 (C) Moriyoshi Koizumi. All rights reserved.
//
// This software is distributed under the Boost Software License, Version 1.0.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef BOOST_PHP_FUNCTION_TEMPLATE_HPP
#define BOOST_PHP_FUNCTION_TEMPLATE_HPP

#include <cstddef>

#include <boost/call_traits.hpp>
#include <boost/preprocessor/config/limits.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>

#include <boost/php/value.hpp>
#include <boost/php/utils.hpp>
#include <boost/php/detail/tsrm_macros.hpp>

namespace boost { namespace php {

class function {
public:
    function(const ::zval& name, value_ptr object)
        : name_(name), object_(object), symtable_(0), fun_table_(0)
          BOOST_PHP_TSRM_FETCH_IN_CTOR_C {}

    function(const value& name, value_ptr object)
        : name_(name), object_(object), symtable_(0), fun_table_(0)
          BOOST_PHP_TSRM_FETCH_IN_CTOR_C {}

    function(const ::zval& name, hashtable<value_ptr>* symtable = 0,
            hashtable< ::zend_function>* fun_table = 0)
        : name_(name), object_(), symtable_(symtable), fun_table_(fun_table)
          BOOST_PHP_TSRM_FETCH_IN_CTOR_C {}

    function(const value& name, hashtable<value_ptr>* symtable = 0,
            hashtable< ::zend_function>* fun_table = 0)
        : name_(name), object_(), symtable_(symtable), fun_table_(fun_table)
          BOOST_PHP_TSRM_FETCH_IN_CTOR_C {}

#define __BOOST_PHP_FUNCTOR_PARAM_ASSIGN_TPL(__z__, __idx__, __var__) \
    value_ptr BOOST_PP_CAT(_arg, __idx__)( \
            to_value_ptr(BOOST_PP_CAT(arg, __idx__) TSRMLS_CC)); \
    __var__[__idx__] = &BOOST_PP_CAT(_arg, __idx__);

#define __BOOST_PHP_FUNCTOR_BODY_TPL(__z__, __arity__, __dummy__) \
{ \
    static const ::std::size_t arity = __arity__; \
    value_ptr* params[arity]; \
    ::zval* retval( 0 ); \
    if (object_->is_null() && !fun_table_) { \
        const_cast<function*>(this)->fun_table_ = \
            reinterpret_cast< hashtable< ::zend_function>*>( \
                CG(function_table)); \
    } \
    BOOST_PP_REPEAT(__arity__, __BOOST_PHP_FUNCTOR_PARAM_ASSIGN_TPL, params) \
    if (FAILURE == call_user_function_ex(fun_table_, \
            object_->is_null() ? NULL: \
                const_cast< ::zval**>( \
                    reinterpret_cast< ::zval* const*>(&*object_)), \
            const_cast<value*>(&name_), &retval, arity, \
            reinterpret_cast< ::zval***>(params), 0, \
            symtable_ TSRMLS_CC)) { \
        throw runtime_error( \
                ::std::string("Unable to call ") \
                + utils::callable_name_to_string(name_ TSRMLS_CC) \
                + "()"); \
    } \
    return value_ptr(retval, false); \
}

#define __BOOST_PHP_FUNCTOR_TPL(__z__, __arity__, __dummy__)  \
template< \
        BOOST_PP_ENUM_PARAMS_Z(__z__, __arity__, typename Targ) \
        > \
value_ptr operator()( \
        BOOST_PP_ENUM_BINARY_PARAMS_Z(__z__, __arity__, Targ, arg)) const \
__BOOST_PHP_FUNCTOR_BODY_TPL(__z__, __arity__, __dummy__)

BOOST_PP_REPEAT_FROM_TO(1, BOOST_PP_LIMIT_MAG, __BOOST_PHP_FUNCTOR_TPL,)

value_ptr operator()() const
__BOOST_PHP_FUNCTOR_BODY_TPL(2, 0, )

#undef __BOOST_PHP_FUNCTOR_TPL

private:
    value name_;
    value_ptr object_;
    hashtable<value_ptr>* symtable_;
    hashtable< ::zend_function>* fun_table_;
    BOOST_PHP_TSRM_MEMBER;
};

} } // namespace boost::php


#endif /* BOOST_PHP_FUNCTION_TEMPLATE_HPP */
