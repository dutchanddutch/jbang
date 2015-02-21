#pragma once
#include "config/init.h"
#include <type_traits>

using namespace std;


// basic type-level logic

using true_t = true_type;
using false_t = false_type;

template< typename... Factors >
using and_t = typename __and_< Factors... >::type;

template< typename... Terms >
using or_t = typename __or_< Terms... >::type;

template< typename Condition >
using not_t = typename __not_< Condition >::type;


// some missing tests

template< typename T >
using is_signed_integral = and_t< is_integral<T>, is_signed<T> >;

template< typename T >
using is_unsigned_integral = and_t< is_integral<T>, not_t< is_signed<T> > >;


// type aliases of trait conditions

#define MAKE_TRAIT_ALIAS_TEMPLATE( trait )		\
	template< typename T, typename Res = void >	\
	using if_##trait##_t = enable_if_t< is_##trait<T>::value, Res >

// While I'm making these aliases I may as well take the opportunity to lay out
// a taxonomy of C++ types.

MAKE_TRAIT_ALIAS_TEMPLATE(  function                          );
MAKE_TRAIT_ALIAS_TEMPLATE(  reference                         );
MAKE_TRAIT_ALIAS_TEMPLATE(     lvalue_reference               );
MAKE_TRAIT_ALIAS_TEMPLATE(     rvalue_reference               );
MAKE_TRAIT_ALIAS_TEMPLATE(  object                            );
MAKE_TRAIT_ALIAS_TEMPLATE(     array                          );
//MAKE_TRAIT_ALIAS_TEMPLATE(     class_type                     );
MAKE_TRAIT_ALIAS_TEMPLATE(        class                       );
MAKE_TRAIT_ALIAS_TEMPLATE(        union                       );
MAKE_TRAIT_ALIAS_TEMPLATE(     scalar                         );
MAKE_TRAIT_ALIAS_TEMPLATE(        enum                        );
MAKE_TRAIT_ALIAS_TEMPLATE(        pointer                     );
MAKE_TRAIT_ALIAS_TEMPLATE(        member_pointer              );
MAKE_TRAIT_ALIAS_TEMPLATE(           member_object_pointer    );
MAKE_TRAIT_ALIAS_TEMPLATE(           member_function_pointer  );

MAKE_TRAIT_ALIAS_TEMPLATE(        arithmetic                  );
MAKE_TRAIT_ALIAS_TEMPLATE(           floating_point           );
MAKE_TRAIT_ALIAS_TEMPLATE(           integral                 );
MAKE_TRAIT_ALIAS_TEMPLATE(              signed_integral       );
MAKE_TRAIT_ALIAS_TEMPLATE(              unsigned_integral     );
//MAKE_TRAIT_ALIAS_TEMPLATE(              bool                  );
//MAKE_TRAIT_ALIAS_TEMPLATE(              character             );
MAKE_TRAIT_ALIAS_TEMPLATE(        null_pointer                );
MAKE_TRAIT_ALIAS_TEMPLATE(  void                              );

// The gap in the list separates the compound types (above the gap) from the
// fundemental types (below the gap).
MAKE_TRAIT_ALIAS_TEMPLATE(  compound     );
MAKE_TRAIT_ALIAS_TEMPLATE(  fundamental  );

// type qualifiers
MAKE_TRAIT_ALIAS_TEMPLATE(  const     );
MAKE_TRAIT_ALIAS_TEMPLATE(  volatile  );

// various properties
MAKE_TRAIT_ALIAS_TEMPLATE(  trivial             );
//MAKE_TRAIT_ALIAS_TEMPLATE(  trivially_copyable  );
MAKE_TRAIT_ALIAS_TEMPLATE(  standard_layout     );
MAKE_TRAIT_ALIAS_TEMPLATE(  pod                 );
MAKE_TRAIT_ALIAS_TEMPLATE(  literal_type        );

// properties of classes ('final' can also apply to unions)
MAKE_TRAIT_ALIAS_TEMPLATE(  empty        );
MAKE_TRAIT_ALIAS_TEMPLATE(  polymorphic  );
MAKE_TRAIT_ALIAS_TEMPLATE(  abstract     );
//MAKE_TRAIT_ALIAS_TEMPLATE(  final        );

// these don't refer just to the true signed/unsigned integral types but they
// classify *all* arithmetic types into signed (-1 < 0) vs unsigned (-1 > 0)
MAKE_TRAIT_ALIAS_TEMPLATE(  signed    );
MAKE_TRAIT_ALIAS_TEMPLATE(  unsigned  );

// more properties

MAKE_TRAIT_ALIAS_TEMPLATE(  default_constructible            );
MAKE_TRAIT_ALIAS_TEMPLATE(  nothrow_default_constructible    );
//MAKE_TRAIT_ALIAS_TEMPLATE(  trivially_default_constructible  );

MAKE_TRAIT_ALIAS_TEMPLATE(  copy_constructible               );
MAKE_TRAIT_ALIAS_TEMPLATE(  nothrow_copy_constructible       );
//MAKE_TRAIT_ALIAS_TEMPLATE(  trivially_copy_constructible     );

MAKE_TRAIT_ALIAS_TEMPLATE(  move_constructible               );
MAKE_TRAIT_ALIAS_TEMPLATE(  nothrow_move_constructible       );
//MAKE_TRAIT_ALIAS_TEMPLATE(  trivially_move_constructible     );

MAKE_TRAIT_ALIAS_TEMPLATE(  copy_assignable                  );
MAKE_TRAIT_ALIAS_TEMPLATE(  nothrow_copy_assignable          );
//MAKE_TRAIT_ALIAS_TEMPLATE(  trivially_copy_assignable        );

MAKE_TRAIT_ALIAS_TEMPLATE(  move_assignable                  );
MAKE_TRAIT_ALIAS_TEMPLATE(  nothrow_move_assignable          );
//MAKE_TRAIT_ALIAS_TEMPLATE(  trivially_move_assignable        );

MAKE_TRAIT_ALIAS_TEMPLATE(  destructible                     );
MAKE_TRAIT_ALIAS_TEMPLATE(  nothrow_destructible             );
//MAKE_TRAIT_ALIAS_TEMPLATE(  trivially_destructible           );


// Remaining ones don't fit the original macro.
// The variadics (if_constructible_t) don't support a non-void result type.

#undef MAKE_TRAIT_ALIAS_TEMPLATE

template< typename T, typename Res = void >
using if_has_virtual_destructor_t =
	enable_if_t< has_virtual_destructor<T>::value, Res >;

#define MAKE_TRAIT_ALIAS_TEMPLATE( trait )		\
	template< typename T, typename... Args >	\
	using if_##trait##_t = enable_if_t< is_##trait< T, Args... >::value >

MAKE_TRAIT_ALIAS_TEMPLATE(  constructible            );
MAKE_TRAIT_ALIAS_TEMPLATE(  nothrow_constructible    );
//MAKE_TRAIT_ALIAS_TEMPLATE(  trivially_constructible  );

#undef MAKE_TRAIT_ALIAS_TEMPLATE

#define MAKE_TRAIT_ALIAS_TEMPLATE( trait )			\
	template< typename T, typename U, typename Res = void >	\
	using if_##trait##_t = enable_if_t< is_##trait< T, U >::value, Res >

MAKE_TRAIT_ALIAS_TEMPLATE(  assignable            );
MAKE_TRAIT_ALIAS_TEMPLATE(  nothrow_assignable    );
//MAKE_TRAIT_ALIAS_TEMPLATE(  trivially_assignable  );

MAKE_TRAIT_ALIAS_TEMPLATE(  same         );
MAKE_TRAIT_ALIAS_TEMPLATE(  base_of      );
MAKE_TRAIT_ALIAS_TEMPLATE(  convertible  );

#undef MAKE_TRAIT_ALIAS_TEMPLATE
