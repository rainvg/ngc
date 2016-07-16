/**
  Angular C core library - Rain

  Released under GNU GENERAL PUBLIC LICENSE

  See also:
  - https://rain.vg
  - https://github.com/rainvg/ngc

  ------------------------------------------

  \file __ngc_initializer__.h

  This file includes the implementation of class \c __ngc_initializer__ and all
  its service nested classes, and the implementation of \c __ngc_initialize__.

  \c __ngc_initializer__ uses introspection and iterators to construct all the
  members of an object. A call to \c __ngc_initialize__ is always the first call
  in the \c __ngc_construct__ method of any class. In other words,
  \c __ngc_initialize__ is used as a delayed version of a member initializer
  list.

  \c __ngc_initialize__ accepts an arbitrary sequence of arguments made by
  compile time strings with the names of the members to initialize followed by
  all the parameters to their constructors.

  For example,

  \code
  myclass() : m1(), m3(1, "hello"), m4(42.42, myobj) {}
  \endcode

  will translate to a call to

  \code
  void __ngc_construct__()
  {
    __ngc_initialize__(*this, ngc :: string <'m', '1'> {}, ngc :: string <'m', '3'> {}, 1, "hello", ngc :: string <'m', '4'> {}, 42.42, myobj);
  }
  \endcode

  See later for further reference.

  \see reference/optional/reference.md
  \see reference/introspection/reference.md
  \see reference/string/reference.md

  \author Matteo Monti [matteo.monti@rain.vg]
  \version 0.0.2
  \date Jul 16, 2016
*/

#ifndef __lib__optional____ngc_factory______ngc_initializer____h
#define __lib__optional____ngc_factory______ngc_initializer____h

#include "../../string/string.h"
#include "__ngc_initializer__.h"

/**
  \class __ngc_initializer__
  \brief A wrapper for all service classes to \c __ngc_initialize__.

  Class \c __ngc_initializer__ wraps all the service classes needed for
  \c __ngc_initialize__ to function properly. The strategy implemented to
  perform initialization is as follows:

   * \c __ngc_member_count__ is used to determine if the object has any member.
   If not, no operation is carried out whatsoever by the initializer.
   * The initializer loops through all the members of the object.
   * The name of each member of the object is retrieved.
   * A service class loops through the arguments provided to the initializer,
   searching for the member name as a string argument.
   * If the member is not found among the arguments, a call to the default
   \c __ngc_construct__ is called on the member.
   * Otherwise, the range of arguments between the member name and either the
   end of the arguments list or the next member name is forwarded to a call to
   \c __ngc_construct__ on the member.

  Each of the steps above is implemented by one specific service nested class
  in \c __ngc_initializer__. See their reference for further details.

  \param type The type of the object to initialize.

  \author Matteo Monti [matteo.monti@rain.vg]
  \version 0.0.1
  \date Jul 16, 2016
*/
template <typename type> struct __ngc_initializer__
{
  /**
    \class clean
    \brief A service class that serves the purpose to remove reference and const
    from a type.

    It does so by exposing a \c ctype typedef with the cleaned type.

    Example usage:

    \code
    clean <int> :: ctype; // int
    clean <int &> :: ctype; // int
    clean <const int> :: ctype; // int
    clean <const int &> :: ctype; // int
    \endcode

    \param dtype The type from which to remove reference and const attributes.

    \author Matteo Monti [matteo.monti@rain.vg]
    \version 0.0.1
    \date Jul 16, 2016
  */
  template <typename dtype> struct clean
  {
    typedef typename std :: remove_const <typename std :: remove_reference <dtype> :: type> :: type ctype; /**< The type without const and reference. */
  };

  /**
    \class is_string
    \brief A service class that, provided with a type template argument,
    determines wether or not that type is an \c ngc \c :: \c string.

    It does so by esposing a static constexpr boolean \c value that is set to
    true if the type is a string, false otherwise.

    Example usage:

    \code
    is_string <int> :: value; // false
    is_string <ngc :: string <'x'>> :: value; // true
    \endcode

    \param stype The type to be diagnosed.

    \author Matteo Monti [matteo.monti@rain.vg]
    \version 0.0.1
    \date Jul 16, 2016
  */
  template <typename stype> struct is_string;

  template <char... chars> struct is_string <ngc :: string <chars...>>
  {
    static constexpr bool value = true; /**< \c true if \c stype is a string, \c false otherwise. */
  };

  template <typename stype> struct is_string
  {
    static constexpr bool value = false; /**< \c true if \c stype is a string, \c false otherwise. */
  };

  /**
    \class arguments_range
    \brief A service class that, provided with a string type and an
    \c __ngc_parameter_pack, loops through the pack to find the position of the
    first occurrence of the string in the pack and the position of the next
    string in the pack.

    It does so by exposing static constexpr size_t \c beg and \c end members
    with values corresponding to the positions of the first occurrence of the
    string \c needle in the parameter pack \c haystack, and of the first string
    after that.

    \c arguments_range also exposes a static constexpr boolean \c found member
    to signal if there was any occurrence of the \c needle in the \c haystack.
    If either the needle or the string next to it is not found, either \c beg or
    \c end will be set to the number of entries in the haystack, i.e., to the
    position of the last possible element plus one.

    \code
    arguments_range <ngc :: string <'a'>, __ngc_parameter_pack__ <>>; // :: beg = 0, :: end = 0
    arguments_range <ngc :: string <'a'>, __ngc_parameter_pack__ <ngc :: string <'a'>>>; // :: beg = 0, :: end = 1
    arguments_range <ngc :: string <'a'>, __ngc_parameter_pack__ <int, ngc :: string <'a'>, int>>; // :: beg = 1, :: end = 3
    arguments_range <ngc :: string <'a'>, __ngc_parameter_pack__ <int, ngc :: string <'a'>, int, ngc :: string <'b'>, char, float, double>>; // :: beg = 1, :: end = 3
    \endcode

    \param needle The string type to search for.
    \param haystack The parameter pack in which to search for the string.

    \author Matteo Monti
    \version 0.0.2
    \date Jul 16, 2016
  */
  template <typename needle, typename haystack> struct arguments_range
  {
    /**
      \class iterator
      \brief A service class to determine the arguments range.

      It recursively iterates over smaller and smaller \ihaystack parameter
      packs, exploring the reversed parameter pack in \c haystack so that the
      last iteration occurs on the first item.

      Values are determined by recurring from the beginning (empty reversed
      parameter pack) to the end (full parameter pack, the first element in
      \c ihasystack is the last in the original \c haystack).

      \param ineedle The needle to look for.
      \param ihaystack The inverted haystack.

      \author Matteo Monti
      \version 0.0.1
      \date Jul 16, 2016
    */
    template <typename ineedle, typename ihaystack> struct iterator;

    template <typename ineedle> struct iterator <ineedle, __ngc_parameter_pack__ <>>
    {
      static constexpr bool found = false; /**< End of the recursion, i.e., beginning of the \c haystack: the needle was not found. */
      static constexpr bool completed = false; /**< End of the recursion, i.e., beginning of the \c haystack: the string following the needle was not found. */

      static constexpr size_t beg = 0; /**< End of the recursion, i.e., beginning of the \c haystack: the needle is not found, setting \c beg position to the size of the pack. */
      static constexpr size_t end = 0; /**< End of the recursion, i.e., beginning of the \c haystack: the needle is not found, setting \c end position to the size of the pack. */
    };

    template <typename ineedle, typename ftype, typename... ftypes> struct iterator <ineedle, __ngc_parameter_pack__ <ftype, ftypes...>>
    {
      static constexpr bool match = std :: is_same <typename clean <ineedle> :: ctype, typename clean <ftype> :: ctype> :: value; /**< \c true if the current element (\c ftype) matches the \c needle exactly, \c false otherwise. */
      static constexpr bool string = is_string <typename clean <ftype> :: ctype> :: value && arguments_range <ineedle, __ngc_parameter_pack__ <ftypes...>> :: found; /**< \c true if the current element (\c ftype) is a string and the \c needle was already found. */

      static constexpr bool found = match || iterator <ineedle, __ngc_parameter_pack__ <ftypes...>> :: found; /**< The \c needle was found if there is a \c match on \c ftype or if the needle was already found. */
      static constexpr bool completed = string || iterator <ineedle, __ngc_parameter_pack__ <ftypes...>> :: completed; /**< The search is complete if there is a \c string match on ftype or if the search was already complete. */

      static constexpr size_t beg = iterator <ineedle, __ngc_parameter_pack__ <ftypes...>> :: beg + !found; /**< The position of the \c needle is equal to the position of the needle in the \c haystack with the last element removed (i.e., the \c ihaystack with the first element removed), plus one if the \c needle was still not found. */
      static constexpr size_t end = iterator <ineedle, __ngc_parameter_pack__ <ftypes...>> :: end + !completed; /**< The position of the string after the \c needle is equal to the position of the string after the \c needle in the \c haystack with the last element removed (i.e., the \c ihaystack with the last element removed), plus one if the string after the \c needle was still not found. */
    };

    static constexpr bool found = iterator <needle, typename __ngc_reverse_parameter_pack__ <haystack> :: type> :: found; /**< \c true if \c needle is found, \c false otherwise. */
    static constexpr size_t beg = iterator <needle, typename __ngc_reverse_parameter_pack__ <haystack> :: type> :: beg; /**< The position of the first occurrence of \c needle in \c haystack. */
    static constexpr size_t end = iterator <needle, typename __ngc_reverse_parameter_pack__ <haystack> :: type> :: end; /**< The position of the first occurence of a string after \c needle in \c haystack. */
  };

  template <size_t back, bool dummy> struct back_step;

  template <bool dummy> struct back_step <0, dummy>
  {
    template <typename mtype, typename... atypes> static inline void execute(mtype & member, atypes && ... arguments)
    {
      __ngc_construct__(member, std :: forward <atypes> (arguments)...);
    }
  };

  template <size_t back, bool dummy> struct back_step
  {
    template <typename mtype, typename atype, typename... atypes> static inline void execute(mtype & member, atype && argument, atypes && ... arguments)
    {
      back_step <back - 1, false> :: execute(member, std :: forward <atypes> (arguments)...);
    }
  };

  template <size_t rotate, size_t back> struct rotate_step;

  template <size_t back> struct rotate_step <0, back>
  {
    template <typename mtype, typename... atypes> static inline void execute(mtype & member, atypes && ... arguments)
    {
      back_step <back, false> :: execute(member, std :: forward <atypes> (arguments)...);
    }
  };

  template <size_t rotate, size_t back> struct rotate_step
  {
    template <typename mtype, typename atype, typename... atypes> static inline void execute(mtype & member, atype && argument, atypes && ... arguments)
    {
      rotate_step <rotate - 1, back> :: execute(member, std :: forward <atypes> (arguments)..., std :: forward <atype> (argument));
    }
  };

  template <size_t front, size_t rotate, size_t back> struct front_step;

  template <size_t rotate, size_t back> struct front_step <0, rotate, back>
  {
    template <typename mtype, typename... atypes> static inline void execute(mtype & member, atypes && ... arguments)
    {
      rotate_step <rotate, back> :: execute(member, std :: forward <atypes> (arguments)...);
    }
  };

  template <size_t front, size_t rotate, size_t back> struct front_step
  {
    template <typename mtype, typename atype, typename... atypes> static inline void execute(mtype & member, atype && argument, atypes && ... arguments)
    {
      front_step <front - 1, rotate, back> :: execute(member, std :: forward <atypes> (arguments)...);
    }
  };

  template <typename name> struct member_initializer
  {
    struct parametric_initializer
    {
      template <typename mtype, typename... atypes> static inline void execute(mtype & member, atypes && ... arguments)
      {
        typedef arguments_range <name, __ngc_parameter_pack__ <atypes...>> range;
        front_step <range :: beg + 1, range :: end - range :: beg - 1, sizeof...(atypes) - range :: end> :: execute(member, std :: forward <atypes> (arguments)...);
      }
    };

    struct default_initializer
    {
      template <typename mtype, typename... atypes> static inline void execute(mtype & member, atypes && ... arguments)
      {
        __ngc_construct__(member);
      }
    };

    template <typename mtype, typename... atypes> static inline void execute(mtype & member, atypes && ... arguments)
    {
      typedef arguments_range <name, __ngc_parameter_pack__ <atypes...>> range;
      std :: conditional <range :: found, parametric_initializer, default_initializer> :: type :: execute(member, std :: forward <atypes> (arguments)...);
    }
  };

  template <size_t index, bool dummy> struct member_iterator;

  template <bool dummy> struct member_iterator <0, dummy>
  {
    template <typename... atypes> static inline void execute(type & that, atypes && ... arguments)
    {
      member_initializer <typename type :: template __ngc_member__ <0, false> :: name> :: execute(type :: template __ngc_member__ <0, false> :: get(that), std :: forward <atypes> (arguments)...);
    }
  };

  template <size_t index, bool dummy> struct member_iterator
  {
    template <typename... atypes> static inline void execute(type & that, atypes && ... arguments)
    {
      member_iterator <index - 1, false> :: execute(that, std :: forward <atypes> (arguments)...);
      member_initializer <typename type :: template __ngc_member__ <index, false> :: name> :: execute(type :: template __ngc_member__ <index, false> :: get(that), std :: forward <atypes> (arguments)...);
    }
  };

  struct null_iterator
  {
    template <typename... atypes> static inline void execute(type & that, atypes && ... arguments)
    {
    }
  };
};

template <typename type, typename... atypes> static inline void __ngc_initialize__(type & that, atypes && ... arguments)
{
  std :: conditional <(__ngc_member_count__ <type> :: value > 0), typename __ngc_initializer__ <type> :: template member_iterator <__ngc_member_count__ <type> :: value - 1, false>, typename __ngc_initializer__ <type> :: null_iterator> :: type :: execute(that, std :: forward <atypes> (arguments)...);
}

#endif
