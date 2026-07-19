#ifndef DYNAMIC_COMPARATOR_BUILDER
#define DYNAMIC_COMPARATOR_BUILDER

#pragma once 
#include <algorithm>

#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>
#include <boost/any.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include "reflection.hpp"


class AscendingSort_t
{
};

class DescendingSort_t
{
};


template <typename Value>
	class DynamicComparatorBuilder
{
	private:
		typedef int (*SortComparator_t)(const Value* x, const Value* y);
		
		enum 
		{
			LESS = -1,
			EQUAL = 0,
			MORE = 1
		};
	private:
		class FieldComparator
		{
			protected:
				boost::shared_ptr<reflection::IMethod> NodeSearchComparator;
				SortComparator_t NodeSortComparator;
				reflection::object_t EmptyObject;
				
			public:
				FieldComparator(boost::shared_ptr<reflection::IMethod> _NodeSearchComparator, SortComparator_t _NodeSortComparator, reflection::object_t _EmptyObject) :
					NodeSearchComparator(_NodeSearchComparator),
					NodeSortComparator(_NodeSortComparator),
					EmptyObject(_EmptyObject)
					{
					}
					
				boost::shared_ptr<reflection::IMethod> GetNodeSearchComparator() const
				{
					return NodeSearchComparator;
				}
				
				SortComparator_t GetNodeSortComparator() const
				{
					return NodeSortComparator;
				}
				
				reflection::object_t GetEmptyObject() const
				{
					return EmptyObject;
				}
		};
	private:
		typedef std::list<boost::shared_ptr<FieldComparator> > Comparators_t;
		typedef typename Comparators_t::const_iterator ComparatorsIterator_t;
		Comparators_t comparators;
	public:
		DynamicComparatorBuilder()
		{		
		}
		
		void Clear()
		{
			comparators.clear();
		}
		
		bool Empty() const
		{
			return comparators.empty();
		}
		
		template <typename T, T Value::* Field>
		void AddField(AscendingSort_t)
		{
			static class FieldComparatorAscending
			{
				public:
					static int SortComparator(const Value* x, const Value* y)
					{
						if (x->*Field < y->*Field)
							return LESS;
						else if (x->*Field > y->*Field)
							return MORE;
							
						return EQUAL;
					}
					
					int SearchComparator(const Value* x, const T searchValue) 
					{
					
						if (x->*Field < searchValue)
							return LESS;
						else if (x->*Field > searchValue)
							return MORE;
							
						return EQUAL;
					}
					
			} EmptyObject;
			
			boost::shared_ptr<reflection::IMethod> method(new (decltype(makeMethodType(&decltype(EmptyObject)::SearchComparator)))
								(&decltype(EmptyObject)::SearchComparator, std::string()));
								
			boost::shared_ptr<FieldComparator> comparator(new FieldComparator(method, &FieldComparatorAscending::SortComparator, &EmptyObject));
			
			comparators.push_back(comparator);
								
		}
		
		template <typename T, T Value::* Field>
		void AddField(DescendingSort_t)
		{
			static class FieldComparatorDescending
			{
				public:
					static int SortComparator(const Value* x, const Value* y)
					{
						if (x->*Field > y->*Field)
							return LESS;
						else if (x->*Field < y->*Field)
							return MORE;
							
						return EQUAL;
					}
					
					int SearchComparator(const Value* x, const T searchValue) 
					{
						if (x->*Field > searchValue)
							return LESS;
						else if (x->*Field < searchValue)
							return MORE;
							
						return EQUAL;
					}
					
			} EmptyObject;
			
			
			boost::shared_ptr<reflection::IMethod> method(new (decltype(makeMethodType(&decltype(EmptyObject)::SearchComparator)))
								(&decltype(EmptyObject)::SearchComparator, std::string()));
								
			boost::shared_ptr<FieldComparator> comparator(new FieldComparator(method, &FieldComparatorDescending::SortComparator, &EmptyObject));

			comparators.push_back(comparator);								
		}
		
		
		bool Less(const Value* x, const Value* y) const
		{
			BOOST_FOREACH(const boost::shared_ptr<FieldComparator>& comparator, comparators)
			{
				int compareResult = comparator->GetNodeSortComparator()(x,y);
				
				if (compareResult < 0)
					return true;
				else if (compareResult > 0)
					return false;
			}	
			
			return false;
		}
		
		template <typename ... args_t>
		int CompareSortFields(const Value* x, const args_t&...args) const
		{
			ComparatorsIterator_t begin = comparators.begin(), end = comparators.end();
			
			return CompareFieldsHelper(x, begin, end, args...);
		}
		
		
	private:
		template <typename first_t, typename ... args_t>
		int CompareFieldsHelper(const Value* x, ComparatorsIterator_t& begin, ComparatorsIterator_t& end, const first_t& first, const args_t&...args)  const		
		{
			if (begin!=end)
			{
				boost::any compareResult; 
				reflection::Parameters_t compareParameters;
				
				compareParameters.push_back(x);
				compareParameters.push_back(first);
				
				const boost::shared_ptr<FieldComparator>& comparator = *begin;
				
	
				comparator->GetNodeSearchComparator()->call(comparator->GetEmptyObject(), compareResult, compareParameters);
				
				int iCompareResult = boost::any_cast<int>(compareResult);
				
				if (iCompareResult!=0)
					return iCompareResult;
					
				return CompareFieldsHelper(x, ++begin, end, args...);				
			} else {
				throw std::logic_error("DynamicComparatorBuilder::CompareSortFields: Too many arguments");
			}
		}
		
		template <typename first_t>
		int CompareFieldsHelper(const Value* x,  ComparatorsIterator_t& begin, ComparatorsIterator_t& end, const first_t& first) const
		{
			if (begin!=end)
			{
				boost::any compareResult; 
				reflection::Parameters_t compareParameters;
				
				compareParameters.push_back(x);
				compareParameters.push_back(first);
				
				const boost::shared_ptr<FieldComparator>& comparator = *begin;
				
	
				comparator->GetNodeSearchComparator()->call(comparator->GetEmptyObject(), compareResult, compareParameters);
				
				int iCompareResult = boost::any_cast<int>(compareResult);
				
				if (iCompareResult!=0)
					return iCompareResult;
					
				if (++begin==end)
					return 0;
					
				throw std::logic_error("DynamicComparatorBuilder::CompareSortFields: Too few arguments, more needed for a deterministic comparison based on known fields");
			} else {
				throw std::logic_error("DynamicComparatorBuilder::CompareSortFields: Too many arguments");
			}
		}
};

#endif
