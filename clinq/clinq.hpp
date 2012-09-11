// ----------------------------------------------------------------------------------------------
// Copyright (c) M�rten R�nge.
// ----------------------------------------------------------------------------------------------
// This source code is subject to terms and conditions of the Microsoft Public License. A 
// copy of the license can be found in the License.html file at the root of this distribution. 
// If you cannot locate the  Microsoft Public License, please send an email to 
// dlr@microsoft.com. By using this source code in any fashion, you are agreeing to be bound 
//  by the terms of the Microsoft Public License.
// ----------------------------------------------------------------------------------------------
// You must not remove this notice, or any other, from this software.
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------
#include <algorithm>
#include <cassert>
#include <iterator>
#include <map>
#include <numeric>
#include <type_traits>
#include <vector>
// ----------------------------------------------------------------------------
#ifdef _MSC_VER 
#   pragma warning (push)
#       pragma warning (disable:4512)
#endif
// ----------------------------------------------------------------------------

#define CLINQ_METHOD 
#define CLINQ_INLINEMETHOD inline

// ----------------------------------------------------------------------------
namespace clinq
{               

    // -------------------------------------------------------------------------

    typedef std::size_t size_type;

    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    // Tedious implementation details of clinq
    // -------------------------------------------------------------------------

    namespace detail
    {

        // -------------------------------------------------------------------------

        size_type const invalid_size = static_cast<size_type>(-1);

        // -------------------------------------------------------------------------

        template<typename TValue>
        struct cleanup_type
        {
            typedef typename std::remove_reference<TValue>::type   type;
        };

        template<typename TRangeBuilder, typename TRange>
        struct get_builtup_type
        {
            static TRangeBuilder get_builder ();
            static TRange get_range ();

            typedef decltype (get_builder ().build (get_range ()))  type;
        };

        template<typename TPredicate, typename TValue>
        struct get_transformed_type
        {
            static TValue                       get_value ();
            static TPredicate                   get_predicate ();

            typedef     decltype (get_predicate ()(get_value ()))   raw_type    ;
            typedef     typename cleanup_type<raw_type>::type       type        ;
        };

        template<typename TArray>
        struct get_array_properties;

        template<typename TValue, int Size>
        struct get_array_properties<TValue[Size]>
        {
            enum
            {
                size = Size,
            };

            typedef typename    cleanup_type<TValue>::type  value_type      ;
            typedef             value_type*                 iterator_type   ;
        };

        // -------------------------------------------------------------------------
        // The generic interface
        // -------------------------------------------------------------------------
        // _range classes:
        //      COPYABLE
        //      MOVEABLE (movesemantics)
        //      typedef                 ...         this_type       ;
        //      typedef                 ...         value_type      ;
        //      value_type front () const
        //      bool next ()
        //      template<typename TRangeBuilder>
        //      typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) const 
        // -------------------------------------------------------------------------
        // _builder classes:
        //      COPYABLE
        //      MOVEABLE (movesemantics)
        //      typedef                 ...         this_type       ;
        //      template<typename TRange>
        //      take_range<TRange> build (TRange range) const
        // -------------------------------------------------------------------------

        template<typename TValueIterator>
        struct from_range
        {
            static TValueIterator get_iterator ();

            typedef                 from_range<TValueIterator>          this_type       ;
            typedef                 TValueIterator                      iterator_type   ;

            typedef                 decltype (*get_iterator ())           raw_value_type  ;
            typedef        typename cleanup_type<raw_value_type>::type  value_type      ;

            iterator_type   const   begin   ;
            iterator_type   const   end     ;

            bool                    start   ;
            iterator_type           current ;


            CLINQ_INLINEMETHOD from_range (
                    iterator_type begin
                ,   iterator_type end
                ) throw ()
                :   begin   (std::move (begin))
                ,   end     (std::move (end))
                ,   start   (true)
            {
            }

            CLINQ_INLINEMETHOD from_range (from_range const & v) throw ()
                :   begin   (v.begin)
                ,   end     (v.end)
                ,   start   (v.start)
                ,   current (v.current)
            {
            }
        
            CLINQ_INLINEMETHOD from_range (from_range && v) throw ()
                :   begin   (std::move (v.begin))
                ,   end     (std::move (v.end))
                ,   start   (std::move (v.start))
                ,   current (std::move (v.current))
            {
            }
        
            template<typename TRangeBuilder>
            CLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) const throw ()   
            {
                return range_builder.build (*this);
            }

            CLINQ_INLINEMETHOD value_type front () const 
            {
                assert (!start);
                assert (current != end);

                return *current;
            }

            CLINQ_INLINEMETHOD bool next () throw ()
            {
                if (start)
                {
                    start = false;
                    current = begin;
                }
                else if (current != end)
                {
                    ++current;
                }

                return current != end;
            }
        };

        // -------------------------------------------------------------------------

        struct int_range
        {
            typedef                 int_range                           this_type       ;
            typedef                 int                                 value_type      ;

            bool                    start   ;
            int                     current ;
            int         const       end     ;

            static int get_current (int begin, int end)
            {
                return begin < end ? begin : end;
            }

            static int get_end (int begin, int end)
            {
                return begin < end ? end : begin;
            }

            CLINQ_INLINEMETHOD int_range (
                    int begin
                ,   int end
                ) throw ()
                :   start   (true)
                ,   current (get_current (begin, end))
                ,   end     (get_end (begin,end))
            {
            }

            CLINQ_INLINEMETHOD int_range (int_range const & v) throw ()
                :   start   (v.start)
                ,   current (v.current)
                ,   end     (v.end)
            {
            }
        
            CLINQ_INLINEMETHOD int_range (int_range && v) throw ()
                :   start   (std::move (v.start))
                ,   current (std::move (v.current))
                ,   end     (std::move (v.end))
            {
            }
        
            template<typename TRangeBuilder>
            CLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) const throw ()   
            {
                return range_builder.build (*this);
            }

            CLINQ_INLINEMETHOD int front () const 
            {
                return current;
            }

            CLINQ_INLINEMETHOD bool next () throw ()
            {
                if (start)
                {
                    start = false;
                }
                else if (current < end)
                {
                    ++current;
                }

                return current < end;
            }
        };

        // -------------------------------------------------------------------------

        struct sorting_range
        {
        };

        template<typename TRange, typename TPredicate>
        struct orderby_range : sorting_range
        {
            typedef                 typename TRange::value_type         value_type      ;

            typedef                 orderby_range<TRange, TPredicate>   this_type       ;
            typedef                 TRange                              range_type      ;
            typedef                 TPredicate                          predicate_type  ;

            range_type              range           ;
            predicate_type  const   predicate       ;
            bool            const   sort_ascending  ;

            size_type               current         ;
            std::vector<value_type> sorted_values   ;    

            CLINQ_INLINEMETHOD orderby_range (
                    range_type      range
                ,   predicate_type  predicate
                ,   bool            sort_ascending
                ) throw ()
                :   range           (std::move (range))
                ,   predicate       (std::move (predicate))
                ,   sort_ascending  (sort_ascending)
                ,   current         (invalid_size)
            {
                static_assert (
                        !std::is_convertible<range_type, sorting_range>::value
                    ,   "ordery may not follow orderby or thenby"
                    );
            }

            CLINQ_INLINEMETHOD orderby_range (orderby_range const & v)
                :   range           (v.range)
                ,   predicate       (v.predicate)
                ,   sort_ascending  (v.sort_ascending)
                ,   current         (v.current)
                ,   sorted_values   (v.sorted_values)
            {
            }

            CLINQ_INLINEMETHOD orderby_range (orderby_range && v) throw ()
                :   range           (std::move (v.range))
                ,   predicate       (std::move (v.predicate))
                ,   sort_ascending  (std::move (v.sort_ascending))
                ,   current         (std::move (v.current))
                ,   sorted_values   (std::move (v.sorted_values))
            {
            }

            CLINQ_INLINEMETHOD value_type forwarding_front () const 
            {
                return range.front ();
            }

            CLINQ_INLINEMETHOD bool forwarding_next ()
            {
                return range.next ();
            }

            CLINQ_INLINEMETHOD bool compare_values (value_type const & l, value_type const & r) const
            {
                if (sort_ascending)
                {
                    return predicate (l) < predicate (r);
                }
                else
                {
                    return predicate (l) > predicate (r);
                }
            }

            template<typename TRangeBuilder>
            CLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) throw ()   
            {
                return range_builder.build (*this);
            }

            CLINQ_INLINEMETHOD value_type front () const 
            {
                return sorted_values[current];
            }

            CLINQ_METHOD bool next ()
            {
                if (current == invalid_size)
                {
                    sorted_values.clear ();

                    while (range.next ())
                    {
                        sorted_values.push_back (range.front ());
                    }

                    if (sorted_values.size () == 0)
                    {
                        return false;
                    }

                    std::sort (
                            sorted_values.begin ()
                        ,   sorted_values.end ()
                        ,   [&] (value_type const & l, value_type const & r)
                            {
                                return this->compare_values (l,r);
                            }
                        );

                    current = 0U;
                    return true;
                }

                if (current < sorted_values.size ())
                {
                    ++current;
                }

                return current < sorted_values.size ();
            }
        };

        template<typename TPredicate>
        struct orderby_builder
        {
            typedef                 orderby_builder<TPredicate> this_type       ;
            typedef                 TPredicate                  predicate_type  ;

            predicate_type  const   predicate       ;
            bool            const   sort_ascending  ;

            CLINQ_INLINEMETHOD explicit orderby_builder (predicate_type predicate, bool sort_ascending) throw ()
                :   predicate       (std::move (predicate))
                ,   sort_ascending  (sort_ascending)
            {
            }

            CLINQ_INLINEMETHOD orderby_builder (orderby_builder const & v)
                :   predicate       (v.predicate)
                ,   sort_ascending  (sort_ascending)
            {
            }

            CLINQ_INLINEMETHOD orderby_builder (orderby_builder && v) throw ()
                :   predicate       (std::move (v.predicate))
                ,   sort_ascending  (std::move (sort_ascending))
            {
            }

            template<typename TRange>
            CLINQ_INLINEMETHOD orderby_range<TRange, TPredicate> build (TRange range) const throw ()
            {
                return orderby_range<TRange, TPredicate>(range, predicate, sort_ascending);
            }

        };

        // -------------------------------------------------------------------------

        template<typename TRange, typename TPredicate>
        struct thenby_range : sorting_range
        {
            typedef                 typename TRange::value_type         value_type      ;

            typedef                 thenby_range<TRange, TPredicate>    this_type       ;
            typedef                 TRange                              range_type      ;
            typedef                 TPredicate                          predicate_type  ;

            range_type              range           ;
            predicate_type  const   predicate       ;
            bool            const   sort_ascending  ;

            size_type               current         ;
            std::vector<value_type> sorted_values   ;    

            CLINQ_INLINEMETHOD thenby_range (
                    range_type      range
                ,   predicate_type  predicate
                ,   bool            sort_ascending
                ) throw ()
                :   range           (std::move (range))
                ,   predicate       (std::move (predicate))
                ,   sort_ascending  (sort_ascending)
                ,   current         (invalid_size)
            {
                static_assert (
                        std::is_convertible<range_type, sorting_range>::value
                    ,   "thenby may only follow orderby or thenby"
                    );
            }

            CLINQ_INLINEMETHOD thenby_range (thenby_range const & v)
                :   range           (v.range)
                ,   predicate       (v.predicate)
                ,   sort_ascending  (v.sort_ascending)
                ,   current         (v.current)
                ,   sorted_values   (v.sorted_values)
            {
            }

            CLINQ_INLINEMETHOD thenby_range (thenby_range && v) throw ()
                :   range           (std::move (v.range))
                ,   predicate       (std::move (v.predicate))
                ,   sort_ascending  (std::move (v.sort_ascending))
                ,   current         (std::move (v.current))
                ,   sorted_values   (std::move (v.sorted_values))
            {
            }

            template<typename TRangeBuilder>
            CLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) throw ()   
            {
                return range_builder.build (*this);
            }

            CLINQ_INLINEMETHOD value_type forwarding_front () const 
            {
                return range.front ();
            }

            CLINQ_INLINEMETHOD bool forwarding_next ()
            {
                return range.next ();
            }

            CLINQ_INLINEMETHOD bool compare_values (value_type const & l, value_type const & r) const
            {
                auto pless = range.compare_values (l,r);
                if (pless)
                {
                    return true;
                }

                auto pgreater = range.compare_values (r,l);
                if (pgreater)
                {
                    return false;
                }

                if (sort_ascending)
                {
                    return predicate (l) < predicate (r);
                }
                else
                {
                    return predicate (l) > predicate (r);
                }
            }

            CLINQ_INLINEMETHOD value_type front () const 
            {
                return sorted_values[current];
            }

            CLINQ_METHOD bool next ()
            {
                if (current == invalid_size)
                {
                    sorted_values.clear ();

                    while (range.forwarding_next ())
                    {
                        sorted_values.push_back (range.forwarding_front ());
                    }

                    if (sorted_values.size () == 0)
                    {
                        return false;
                    }

                    std::sort (
                            sorted_values.begin ()
                        ,   sorted_values.end ()
                        ,   [&] (value_type const & l, value_type const & r)
                            {
                                return this->compare_values (l,r);
                            }
                        );

                    current = 0U;
                    return true;
                }

                if (current < sorted_values.size ())
                {
                    ++current;
                }

                return current < sorted_values.size ();
            }
        };

        template<typename TPredicate>
        struct thenby_builder
        {
            typedef                 thenby_builder<TPredicate>  this_type       ;
            typedef                 TPredicate                  predicate_type  ;

            predicate_type  const   predicate       ;
            bool            const   sort_ascending  ;

            CLINQ_INLINEMETHOD explicit thenby_builder (predicate_type predicate, bool sort_ascending) throw ()
                :   predicate       (std::move (predicate))
                ,   sort_ascending  (sort_ascending)
            {
            }

            CLINQ_INLINEMETHOD thenby_builder (thenby_builder const & v)
                :   predicate       (v.predicate)
                ,   sort_ascending  (sort_ascending)
            {
            }

            CLINQ_INLINEMETHOD thenby_builder (thenby_builder && v) throw ()
                :   predicate       (std::move (v.predicate))
                ,   sort_ascending  (std::move (sort_ascending))
            {
            }

            template<typename TRange>
            CLINQ_INLINEMETHOD thenby_range<TRange, TPredicate> build (TRange range) const throw ()
            {
                return thenby_range<TRange, TPredicate>(range, predicate, sort_ascending);
            }

        };

        // -------------------------------------------------------------------------

        template<typename TRange, typename TPredicate>
        struct where_range
        {
            typedef                 typename TRange::value_type     value_type      ;

            typedef                 where_range<TRange, TPredicate> this_type       ;
            typedef                 TRange                          range_type      ;
            typedef                 TPredicate                      predicate_type  ;

            range_type              range       ;
            predicate_type  const   predicate   ;

            CLINQ_INLINEMETHOD where_range (
                    range_type      range
                ,   predicate_type  predicate
                ) throw ()
                :   range       (std::move (range))
                ,   predicate   (std::move (predicate))
            {
            }

            CLINQ_INLINEMETHOD where_range (where_range const & v)
                :   range       (v.range)
                ,   predicate   (v.predicate)
            {
            }

            CLINQ_INLINEMETHOD where_range (where_range && v) throw ()
                :   range       (std::move (v.range))
                ,   predicate   (std::move (v.predicate))
            {
            }

            template<typename TRangeBuilder>
            CLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) throw ()   
            {
                return range_builder.build (*this);
            }

            CLINQ_INLINEMETHOD value_type front () const 
            {
                return range.front ();
            }

            CLINQ_INLINEMETHOD bool next ()
            {
                while (range.next ())
                {
                    if (predicate (range.front ()))
                    {
                        return true;
                    }
                }

                return false;
            }
        };

        template<typename TPredicate>
        struct where_builder
        {
            typedef                 where_builder<TPredicate>   this_type       ;
            typedef                 TPredicate                  predicate_type  ;

            predicate_type  const   predicate   ;

            CLINQ_INLINEMETHOD explicit where_builder (predicate_type predicate) throw ()
                :   predicate (std::move (predicate))
            {
            }

            CLINQ_INLINEMETHOD where_builder (where_builder const & v)
                :   predicate (v.predicate)
            {
            }

            CLINQ_INLINEMETHOD where_builder (where_builder && v) throw ()
                :   predicate (std::move (v.predicate))
            {
            }

            template<typename TRange>
            CLINQ_INLINEMETHOD where_range<TRange, TPredicate> build (TRange range) const throw ()
            {
                return where_range<TRange, TPredicate>(range, predicate);
            }

        };

        // -------------------------------------------------------------------------

        // -------------------------------------------------------------------------

        template<typename TRange>
        struct take_range
        {
            typedef                 typename TRange::value_type     value_type      ;

            typedef                 take_range<TRange>              this_type       ;
            typedef                 TRange                          range_type      ;

            range_type              range       ;
            size_type   const       count       ;
            size_type               current     ;


            CLINQ_INLINEMETHOD take_range (
                    range_type      range
                ,   size_type       count
                ) throw ()
                :   range       (std::move (range))
                ,   count       (std::move (count))
                ,   current     (0)         
            {
            }

            CLINQ_INLINEMETHOD take_range (take_range const & v)
                :   range       (v.range)
                ,   count       (v.count)
                ,   current     (v.current)
            {
            }

            CLINQ_INLINEMETHOD take_range (take_range && v) throw ()
                :   range       (std::move (v.range))
                ,   count       (std::move (v.count))
                ,   current     (std::move (v.current))
            {
            }

            template<typename TRangeBuilder>
            CLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) const throw ()   
            {
                return range_builder.build (*this);
            }

            CLINQ_INLINEMETHOD value_type front () const 
            {
                return range.front ();
            }

            CLINQ_INLINEMETHOD bool next ()
            {
                if (current < count)
                {
                    ++current;
                    return range.next ();
                }

                return false;
            }
        };

        struct take_builder
        {
            typedef                 take_builder        this_type       ;

            size_type   const       count       ;

            CLINQ_INLINEMETHOD explicit take_builder (size_type count) throw ()
                :   count (std::move (count))
            {
            }

            CLINQ_INLINEMETHOD take_builder (take_builder const & v) throw ()
                :   count (v.count)
            {
            }

            CLINQ_INLINEMETHOD take_builder (take_builder && v) throw ()
                :   count (std::move (v.count))
            {
            }

            template<typename TRange>
            CLINQ_INLINEMETHOD take_range<TRange> build (TRange range) const throw ()
            {
                return take_range<TRange>(range, count);
            }

        };

        // -------------------------------------------------------------------------

        template<typename TRange>
        struct skip_range
        {
            typedef                 typename TRange::value_type     value_type      ;

            typedef                 skip_range<TRange>              this_type       ;
            typedef                 TRange                          range_type      ;

            range_type              range       ;
            size_type   const       count       ;
            size_type               current     ;

            CLINQ_INLINEMETHOD skip_range (
                    range_type      range
                ,   size_type       count
                ) throw ()
                :   range       (std::move (range))
                ,   count       (std::move (count))
                ,   current     (0)
            {
            }

            CLINQ_INLINEMETHOD skip_range (skip_range const & v)
                :   range       (v.range)
                ,   count       (v.count)
                ,   current     (v.current)
            {
            }

            CLINQ_INLINEMETHOD skip_range (skip_range && v) throw ()
                :   range       (std::move (v.range))
                ,   count       (std::move (v.count))
                ,   current     (std::move (v.current))
            {
            }

            template<typename TRangeBuilder>
            CLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) const throw ()   
            {
                return range_builder.build (*this);
            }

            CLINQ_INLINEMETHOD value_type front () const 
            {
                return range.front ();
            }

            CLINQ_INLINEMETHOD bool next ()
            {
                if (current == invalid_size)
                {
                    return false;
                }

                while (current < count && range.next ())
                {
                    ++current;              
                }

                if (current < count)
                {
                    current = invalid_size;
                    return false;
                }

                return range.next ();
            }
        };

        struct skip_builder
        {
            typedef                 skip_builder        this_type       ;

            size_type   const       count       ;

            CLINQ_INLINEMETHOD explicit skip_builder (size_type count) throw ()
                :   count (std::move (count))
            {
            }

            CLINQ_INLINEMETHOD skip_builder (skip_builder const & v) throw ()
                :   count (v.count)
            {
            }

            CLINQ_INLINEMETHOD skip_builder (skip_builder && v) throw ()
                :   count (std::move (v.count))
            {
            }

            template<typename TRange>
            CLINQ_INLINEMETHOD skip_range<TRange> build (TRange range) const throw ()
            {
                return skip_range<TRange>(range, count);
            }

        };

        // -------------------------------------------------------------------------

        template<typename TRange, typename TPredicate>
        struct select_range
        {
            static typename TRange::value_type get_source ();
            static          TPredicate get_predicate ();


            typedef         decltype (get_predicate ()(get_source ()))     raw_value_type  ;
            typedef        typename cleanup_type<raw_value_type>::type  value_type      ;

            typedef                 select_range<TRange, TPredicate>    this_type       ;
            typedef                 TRange                              range_type      ;
            typedef                 TPredicate                          predicate_type  ;

            range_type              range       ;
            predicate_type  const   predicate   ;

            CLINQ_INLINEMETHOD select_range (
                    range_type      range
                ,   predicate_type  predicate
                ) throw ()
                :   range       (std::move (range))
                ,   predicate   (std::move (predicate))
            {
            }

            CLINQ_INLINEMETHOD select_range (select_range const & v)
                :   range       (v.range)
                ,   predicate   (v.predicate)
            {
            }

            CLINQ_INLINEMETHOD select_range (select_range && v) throw ()
                :   range       (std::move (v.range))
                ,   predicate   (std::move (v.predicate))
            {
            }

            template<typename TRangeBuilder>
            CLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) const throw ()   
            {
                return range_builder.build (*this);
            }

            CLINQ_INLINEMETHOD value_type front () const 
            {
                return predicate (range.front ());
            }

            CLINQ_INLINEMETHOD bool next ()
            {
                return range.next ();
            }
        };

        template<typename TPredicate>
        struct select_builder
        {
            typedef                 select_builder<TPredicate>  this_type       ;
            typedef                 TPredicate                  predicate_type  ;

            predicate_type  const   predicate   ;

            CLINQ_INLINEMETHOD explicit select_builder (predicate_type predicate) throw ()
                :   predicate (std::move (predicate))
            {
            }

            CLINQ_INLINEMETHOD select_builder (select_builder const & v)
                :   predicate (v.predicate)
            {
            }

            CLINQ_INLINEMETHOD select_builder (select_builder && v) throw ()
                :   predicate (std::move (v.predicate))
            {
            }

            template<typename TRange>
            CLINQ_INLINEMETHOD select_range<TRange, TPredicate> build (TRange range) const throw ()
            {
                return select_range<TRange, TPredicate>(range, predicate);
            }

        };

        // -------------------------------------------------------------------------

        // TODO: The way this iterator is done has two problems
        // 1. operator-> is not implemented due to technical difficulties
        //    Investigate how boost does this
        // 2. The iterator modifies the underlying range, a couple of reasons for that:
        //      1. It's not certain the range is cheaply copyable and iterators should be cheap to copy
        //      2. It's not certain the underlying collection supports iterating over it multiple times

        template<typename TRange>
        struct container_iterator
        {
            typedef                 std::forward_iterator_tag   iterator_category   ;
            typedef     typename    TRange::value_type          value_type          ;
            typedef                 std::ptrdiff_t              difference_type     ;
            typedef                 value_type*                 pointer             ;
	        typedef                 value_type&                 reference           ;

            typedef                 container_iterator<TRange>  this_type   ;
            typedef                 TRange                      range_type  ;

            bool                    is_at_end                               ;
            range_type* const       prange                                  ;

            CLINQ_INLINEMETHOD container_iterator ()   throw ()
                :   is_at_end   (true)
                ,   prange      (nullptr)
            {
            }

            CLINQ_INLINEMETHOD container_iterator (range_type * prange)   throw ()
                :   is_at_end   (prange ? !prange->next () : true)
                ,   prange      (prange)
            {
            }

            CLINQ_INLINEMETHOD container_iterator (container_iterator const & v) throw ()
                :   is_at_end   (v.is_at_end)
                ,   prange      (v.prange)
            {
            }

            CLINQ_INLINEMETHOD container_iterator (container_iterator && v) throw ()
                :   is_at_end   (std::move (v.is_at_end))
                ,   prange      (std::move (v.prange))
            {
            }

            CLINQ_INLINEMETHOD value_type  operator* () const throw ()
            {
                assert (!is_at_end);
                assert (prange);
                return prange->front ();
            }

            // TODO: operator-> but this is complicated by the fact that front ()
            // returns a value

            CLINQ_INLINEMETHOD this_type & operator++()
            {
                if (!is_at_end && prange)
                {
                    is_at_end = !prange->next ();
                }

                return *this;
            }

            CLINQ_INLINEMETHOD bool operator== (this_type const & v) const throw ()
            {
                if (is_at_end && v.is_at_end)
                {
                    return true;
                }
                else if (!is_at_end && !v.is_at_end && prange == v.prange)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }

            CLINQ_INLINEMETHOD bool operator!= (this_type const & v) const throw ()
            {
                return !(*this == v);
            }
        };

        template<typename TRange>
        struct container
        {
            typedef                 container<TRange>   this_type   ;
            typedef                 TRange              range_type  ;
            typedef     typename    TRange::value_type  value_type  ;

            range_type              range   ;

            CLINQ_INLINEMETHOD explicit container (TRange range)
                :   range (range)
            {
            }

            CLINQ_INLINEMETHOD container (container const & v) throw ()
                :   range       (v.range)
            {
            }

            CLINQ_INLINEMETHOD container (container && v) throw ()
                :   range       (std::move (v.range))
            {
            }

            CLINQ_INLINEMETHOD container_iterator<TRange>  begin () throw ()
            {
                return container_iterator<TRange>(std::addressof (range));
            }

            CLINQ_INLINEMETHOD container_iterator<TRange>  end () throw ()
            {
                return container_iterator<TRange>();
            }

        };

        struct container_builder
        {
            typedef                 container_builder       this_type       ;

            CLINQ_INLINEMETHOD container_builder () throw ()
            {
            }

            CLINQ_INLINEMETHOD container_builder (container_builder const & v) throw ()
            {
            }

            CLINQ_INLINEMETHOD container_builder (container_builder && v) throw ()
            {
            }

            template<typename TRange>
            CLINQ_METHOD container<TRange> build (TRange range)
            {
                return container<TRange> (range);
            }

        };

        // -------------------------------------------------------------------------

        struct to_vector_builder
        {
            typedef                 to_vector_builder       this_type       ;

            size_type   const       capacity;

            CLINQ_INLINEMETHOD explicit to_vector_builder (size_type capacity = 16U) throw ()
                :   capacity    (capacity)
            {
            }

            CLINQ_INLINEMETHOD to_vector_builder (to_vector_builder const & v) throw ()
                :   capacity (v.capacity)
            {
            }

            CLINQ_INLINEMETHOD to_vector_builder (to_vector_builder && v) throw ()
                :   capacity (std::move (v.capacity))
            {
            }

            template<typename TRange>
            CLINQ_METHOD std::vector<typename TRange::value_type> build (TRange range)
            {
                std::vector<typename TRange::value_type> result;

                while (range.next ())
                {
                    result.push_back (range.front ());
                }

                return std::move (result);
            }

        };

        // -------------------------------------------------------------------------

        template<typename TKeyPredicate>
        struct to_map_builder
        {
            static TKeyPredicate get_key_predicate ();

            typedef                     to_map_builder<TKeyPredicate>  this_type           ;
            typedef                     TKeyPredicate               key_predicate_type  ;

            key_predicate_type  const   key_predicate   ;

            CLINQ_INLINEMETHOD explicit to_map_builder (key_predicate_type key_predicate) throw ()
                :   key_predicate   (key_predicate)
            {
            }

            CLINQ_INLINEMETHOD to_map_builder (to_map_builder const & v)
                :   key_predicate (v.key_predicate)
            {
            }

            CLINQ_INLINEMETHOD to_map_builder (to_map_builder && v) throw ()
                :   key_predicate (std::move (v.key_predicate))
            {
            }

            template<typename TRange>
            CLINQ_METHOD std::map<
                    typename get_transformed_type<key_predicate_type, typename TRange::value_type>::type
                ,   typename TRange::value_type
                > build (TRange range)
            {
                std::map<
                    typename get_transformed_type<key_predicate_type, typename TRange::value_type>::type
                ,   typename TRange::value_type
                >   result;

                while (range.next ())
                {
                    auto v = range.front ();
                    result[key_predicate (v)] = v;
                }

                return std::move (result);
            }

        };

        // -------------------------------------------------------------------------

        template<typename TPredicate>
        struct for_each_builder
        {
            typedef                 for_each_builder<TPredicate>    this_type       ;
            typedef                 TPredicate                      predicate_type  ;

            predicate_type  const   predicate;

            CLINQ_INLINEMETHOD explicit for_each_builder (predicate_type predicate) throw ()
                :   predicate    (predicate)
            {
            }

            CLINQ_INLINEMETHOD for_each_builder (for_each_builder const & v) throw ()
                :   predicate (v.predicate)
            {
            }

            CLINQ_INLINEMETHOD for_each_builder (for_each_builder && v) throw ()
                :   predicate (std::move (v.predicate))
            {
            }


            template<typename TRange>
            CLINQ_INLINEMETHOD void build (TRange range)
            {
                while (range.next ())
                {
                    predicate (range.front ());
                }
            }

        };

        // -------------------------------------------------------------------------

        struct first_builder
        {
            typedef                 first_builder                   this_type       ;

            CLINQ_INLINEMETHOD first_builder () throw ()
            {
            }

            CLINQ_INLINEMETHOD first_builder (first_builder const & v) throw ()
            {
            }

            CLINQ_INLINEMETHOD first_builder (first_builder && v) throw ()
            {
            }

            template<typename TRange>
            CLINQ_INLINEMETHOD typename TRange::value_type build (TRange range)
            {
                if (range.next ())
                {
                    return range.front ();
                }

                return typename TRange::value_type ();
            }

        };

        // -------------------------------------------------------------------------

        struct count_builder
        {
            typedef                 count_builder                   this_type       ;

            CLINQ_INLINEMETHOD count_builder () throw ()
            {
            }

            CLINQ_INLINEMETHOD count_builder (count_builder const & v) throw ()
            {
            }

            CLINQ_INLINEMETHOD count_builder (count_builder && v) throw ()
            {
            }


            template<typename TRange>
            CLINQ_INLINEMETHOD size_type build (TRange range)
            {
                size_type count = 0U;
                while (range.next ())
                {
                    ++count;
                }
                return count;
            }

        };

        // -------------------------------------------------------------------------

        struct sum_builder
        {
            typedef                 sum_builder                     this_type       ;

            CLINQ_INLINEMETHOD sum_builder () throw ()
            {
            }

            CLINQ_INLINEMETHOD sum_builder (sum_builder const & v) throw ()
            {
            }

            CLINQ_INLINEMETHOD sum_builder (sum_builder && v) throw ()
            {
            }

            template<typename TRange>
            CLINQ_INLINEMETHOD typename TRange::value_type build (TRange range)
            {
                auto sum = typename TRange::value_type ();
                while (range.next ())
                {
                    sum += range.front ();
                }
                return sum;
            }

        };

        // -------------------------------------------------------------------------

        struct max_builder
        {
            typedef                 max_builder                         this_type       ;

            CLINQ_INLINEMETHOD max_builder () throw ()
            {
            }

            CLINQ_INLINEMETHOD max_builder (max_builder const & v) throw ()
            {
            }

            CLINQ_INLINEMETHOD max_builder (max_builder && v) throw ()
            {
            }


            template<typename TRange>
            CLINQ_INLINEMETHOD typename TRange::value_type build (TRange range)
            {
                auto current = std::numeric_limits<typename TRange::value_type>::min ();
                while (range.next ())
                {
                    auto v = range.front ();
                    if (current < v)
                    {
                        current = v;
                    }
                }
                return current;
            }

        };

        // -------------------------------------------------------------------------

        struct min_builder
        {
            typedef                 min_builder                         this_type       ;

            CLINQ_INLINEMETHOD min_builder () throw ()
            {
            }

            CLINQ_INLINEMETHOD min_builder (min_builder const & v) throw ()
            {
            }

            CLINQ_INLINEMETHOD min_builder (min_builder && v) throw ()
            {
            }


            template<typename TRange>
            CLINQ_INLINEMETHOD typename TRange::value_type build (TRange range)
            {
                auto current = std::numeric_limits<typename TRange::value_type>::max ();
                while (range.next ())
                {
                    auto v = range.front ();
                    if (current > v)
                    {
                        current = v;
                    }
                }
                return current;
            }

        };

        // -------------------------------------------------------------------------

    }   // namespace detail

    // -------------------------------------------------------------------------
    // The interface of clinq
    // -------------------------------------------------------------------------

    template<typename TValueIterator>
    CLINQ_INLINEMETHOD detail::from_range<TValueIterator> from_iterators (
            TValueIterator  begin
        ,   TValueIterator  end
        ) throw ()
    {
        return detail::from_range<TValueIterator> (begin,end);
    }

    template<typename TContainer>
    CLINQ_INLINEMETHOD detail::from_range<typename TContainer::const_iterator> from (
            TContainer  const & container
        ) throw ()
    {
        return detail::from_range<typename TContainer::const_iterator> (
                container.begin ()
            ,   container.end ()
            );
    }

    template<typename TValueArray>
    CLINQ_INLINEMETHOD detail::from_range<typename detail::get_array_properties<TValueArray>::iterator_type> from_array (
            TValueArray & a 
        ) throw ()
    {
        typedef detail::get_array_properties<TValueArray>   array_properties;
        return detail::from_range<typename array_properties::iterator_type> (
                a
            ,   a   +   array_properties::size
            );
    }

    CLINQ_INLINEMETHOD detail::int_range range (
            int         start
        ,   int         count
        ) throw ()
    {
        auto c      = count > 0 ? count : 0;
        auto end    = (INT_MAX - c) > start ? (start + c) : INT_MAX;
        return detail::int_range (start, start + end);
    }

    template<typename TPredicate>
    CLINQ_INLINEMETHOD detail::orderby_builder<TPredicate> orderby (
            TPredicate      predicate
        ,   bool            sort_ascending  = true
        ) throw ()
    {
        return detail::orderby_builder<TPredicate> (predicate, sort_ascending);
    }

    template<typename TPredicate>
    CLINQ_INLINEMETHOD detail::thenby_builder<TPredicate> thenby (
            TPredicate      predicate
        ,   bool            sort_ascending  = true
        ) throw ()
    {
        return detail::thenby_builder<TPredicate> (predicate, sort_ascending);
    }

    template<typename TPredicate>
    CLINQ_INLINEMETHOD detail::where_builder<TPredicate> where (
            TPredicate      predicate
        ) throw ()
    {
        return detail::where_builder<TPredicate> (predicate);
    }

    CLINQ_INLINEMETHOD detail::take_builder take (
            size_type     count
        ) throw ()
    {
        return detail::take_builder (count);
    }

    CLINQ_INLINEMETHOD detail::skip_builder skip (
            size_type       count
        ) throw ()
    {
        return detail::skip_builder (count);
    }

    template<typename TPredicate>
    CLINQ_INLINEMETHOD detail::select_builder<TPredicate> select (
            TPredicate      predicate
        ) throw ()
    {
        return detail::select_builder<TPredicate> (predicate);
    }

    CLINQ_INLINEMETHOD detail::container_builder    container () throw ()
    {
        return detail::container_builder ();
    }

    CLINQ_INLINEMETHOD detail::to_vector_builder    to_vector () throw ()
    {
        return detail::to_vector_builder ();
    }

    template<typename TKeyPredicate>
    CLINQ_INLINEMETHOD detail::to_map_builder<TKeyPredicate>  to_map (TKeyPredicate key_predicate) throw ()
    {
        return detail::to_map_builder<TKeyPredicate>(key_predicate);
    }

    template<typename TPredicate>
    CLINQ_INLINEMETHOD detail::for_each_builder<TPredicate> for_each (
            TPredicate predicate
        ) throw ()
    {
        return detail::for_each_builder<TPredicate> (predicate);
    }

    CLINQ_INLINEMETHOD detail::first_builder   first () throw ()
    {
        return detail::first_builder ();
    }

    CLINQ_INLINEMETHOD detail::count_builder   count () throw ()
    {
        return detail::count_builder ();
    }

    CLINQ_INLINEMETHOD detail::sum_builder  sum () throw ()
    {
        return detail::sum_builder ();
    }

    CLINQ_INLINEMETHOD detail::max_builder  max () throw ()
    {
        return detail::max_builder ();
    }

    CLINQ_INLINEMETHOD detail::min_builder  min () throw ()
    {
        return detail::min_builder ();
    }

    // -------------------------------------------------------------------------

}
// ----------------------------------------------------------------------------
#ifdef _MSC_VER 
#   pragma warning (pop)
#endif
// ----------------------------------------------------------------------------