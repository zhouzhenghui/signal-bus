/*
 * Copyright 2009, 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __PP_REMOVE_PARENS_H
#define __PP_REMOVE_PARENS_H

#include <boost/preprocessor/variadic.hpp>
#include <boost/preprocessor/control.hpp>
#include <boost/preprocessor/facilities/expand.hpp>
#include <preprocessor/is_empty.h>

#if BOOST_PP_VARIADICS
#  define __PP_IN_PARENS(...) __PP_IS_EMPTY(__PP_REMOVE_PARENS_ELEM __VA_ARGS__)
#  define __PP_REMOVE_PARENS_ELEM(...)
#  define __PP_REMOVE_PARENS_D(...) __VA_ARGS__
#else
#  define __PP_IN_PARENS(a) __PP_IS_EMPTY(__PP_REMOVE_PARENS_ELEM a)
#  define __PP_REMOVE_PARENS_ELEM(_)
#  define __PP_REMOVE_PARENS_D(a) a
#endif /* BOOST_PP_VARIADICS */

#define __PP_REMOVE_PARENS_PRED(d, a) __PP_IN_PARENS(__PP_REMOVE_PARENS_D a)
#define __PP_REMOVE_PARENS_OP(d, a) __PP_REMOVE_PARENS_D a
#define __PP_REMOVE_PARENS(a) BOOST_PP_EXPAND(__PP_REMOVE_PARENS_D BOOST_PP_WHILE(__PP_REMOVE_PARENS_PRED, __PP_REMOVE_PARENS_OP, (a)))

/*
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/tuple.hpp>
#include <boost/preprocessor/control.hpp>
#include <boost/preprocessor/comparison.hpp>
#include <boost/preprocessor/logical.hpp>
#include <preprocessor/is_empty.h>

#define __PP_REMOVE_PARENS_PRED(d, tuple) BOOST_PP_TUPLE_ELEM(3, 0, tuple)
#define __PP_REMOVE_PARENS_OP(d, tuple) __PP_REMOVE_PARENS_OP_D(__PP_REMOVE_PARENS_OP_SEQ(BOOST_PP_TUPLE_ELEM(3, 1, tuple)), BOOST_PP_TUPLE_ELEM(3, 2, tuple))
#define __PP_REMOVE_PARENS_OP_SEQ(ref) BOOST_PP_CAT(__PP_REMOVE_PARENS_OP_DEF_, __PP_REMOVE_PARENS_OP_HELPER ref) )
#define __PP_REMOVE_PARENS_OP_HELPER(ref) 00(ref)
#define __PP_REMOVE_PARENS_OP_DEF_00(x) (x)(
#define __PP_REMOVE_PARENS_OP_DEF___PP_REMOVE_PARENS_OP_HELPER (
#define __PP_REMOVE_PARENS_OP_D(seq, orig) \
    (__PP_IS_EMPTY(BOOST_PP_SEQ_HEAD(BOOST_PP_SEQ_REVERSE(seq))) \
        , BOOST_PP_IF(BOOST_PP_OR(__PP_IS_EMPTY(BOOST_PP_SEQ_HEAD(BOOST_PP_SEQ_REVERSE(seq))), BOOST_PP_EQUAL(BOOST_PP_SEQ_SIZE(seq), 1)) \
            , BOOST_PP_SEQ_HEAD(seq) \
            , (BOOST_PP_SEQ_HEAD(seq)) BOOST_PP_SEQ_HEAD(BOOST_PP_SEQ_REVERSE(seq))) \
        , orig)
#define __PP_REMOVE_PARENS(ref) \
  BOOST_PP_TUPLE_ELEM(3, 1, BOOST_PP_WHILE(__PP_REMOVE_PARENS_PRED, __PP_REMOVE_PARENS_OP, (1, ref, ref)))
*/
#endif /* __PP_REMOVE_PARENS_H */
