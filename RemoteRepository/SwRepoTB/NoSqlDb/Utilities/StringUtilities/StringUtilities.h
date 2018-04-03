#ifndef STRINGUTILITIES_H
#define STRINGUTILITIES_H
///////////////////////////////////////////////////////////////////////
// StringUtilities.h - small, generally useful, helper classes       //
// ver 1.0                                                           //
// Language:    C++, Visual Studio 2017                              //
// Application: Most Projects, CSE687 - Object Oriented Design       //
// Author:      Jim Fawcett, Syracuse University, CST 4-187          //
//              jfawcett@twcny.rr.com                                //
//				Tianyu Qi (modified this package and added functions)//
//				tqi100@syr.edu										 //
///////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides functions:
* - Title(text)           display title
* - title(text)           display subtitle
* - putline(n)            display n newlines
* - trim(str)             remove leading and trailing whitespace
* - split(str, 'delim')   break string into vector of strings separated by delim char 
* - showSplit(vector)     display splits
* - splitPlus(toSplit, splitOn) modified split function, skip splitOn inside '[]'
* - unwrapper(toWrap)	  return string without wrapper
* - wrapperPlus(toUnwrap, wrapper) modified unwrapper function, integrated checking 
* - checkWrapper(toCheck, wrapper) check if string is wrapped correctly
* - regexSafeFilter(toFilter) Escape all metacharacter in regex string.
*
* Required Files:
* ---------------
*   StringUtilities.h
*
* Maintenance History:
* --------------------
* ver 1.0 : 12 Jan 2018
* - first release
* - refactored from earlier Utilities.h
* ver 1.1 : 01 Feb 2018
* - add showSplit, unwrapper, wrapperPlus, checkWrapper
* - considered useful in Project 1.
* ver 1.2 : 27 Feb 2018
* - add regexSafeFileter
*
* Notes:
* ------
* - Designed to provide all functionality in header file.
* - Implementation file only needed for test and demo.
*
* Planned Additions and Changes:
* ------------------------------
* - none yet
*/
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <functional>
#include <locale>

namespace Utilities
{
  /////////////////////////////////////////////////////////////////////
  // String Helper functions
 
  //----< display underlined title >-----------------------------------

  inline void Title(const std::string& text, std::ostream& out = std::cout, char underline = '=')
  {
    out << "\n  " << text;
    out << "\n " << std::string(text.size() + 2, underline);
  }
  //----< display underlined subtitle >--------------------------------

  inline void title(const std::string& text, std::ostream& out = std::cout, char underline = '-')
  {
    out << "\n  " << text;
    out << "\n " << std::string(text.size() + 2, underline);
  }
  //----< display j newlines >-----------------------------------------

  inline void putline(size_t j = 1, std::ostream& out = std::cout)
  {
    for (size_t i = 0; i < j; ++i)
      out << "\n";
  }
  /*--- remove whitespace from front and back of string argument ---*/
  /*
  *  - does not remove newlines
  */
  template <typename T>
  inline std::basic_string<T> trim(const std::basic_string<T>& toTrim)
  {
    if (toTrim.size() == 0)
      return toTrim;
    std::basic_string<T> temp;
    std::locale loc;
    typename std::basic_string<T>::const_iterator iter = toTrim.begin();
    while (isspace(*iter, loc) && *iter != '\n')
    {
      if (++iter == toTrim.end())
      {
        break;
      }
    }
    for (; iter != toTrim.end(); ++iter)
    {
      temp += *iter;
    }
    typename std::basic_string<T>::reverse_iterator riter;
    size_t pos = temp.size();
    for (riter = temp.rbegin(); riter != temp.rend(); ++riter)
    {
      --pos;
      if (!isspace(*riter, loc) || *riter == '\n')
      {
        break;
      }
    }
    if (0 <= pos && pos < temp.size())
      temp.erase(++pos);
    return temp;
  }

  // Remove the customed wrapping character at head and tail of the string.
  template <typename T>
  inline std::basic_string<T> unwrapper(const std::basic_string<T>& toUnwrap)
  {
	  return toUnwrap.substr(1, toUnwrap.length() - 2);
  }

  // Modified version of original unwrapper, integrated unwrappable checking.
  // Reject wrapper except "\"", "\/", "()", "[]", "{}"
  template <typename T>
  inline std::basic_string<T> unwrapPlus(const std::basic_string<T>& toUnwrap, T wrapper = '\"') {
	  if (toUnwrap.size() == 0) throw std::exception("Attempt unwrap empty string.\n");
	  if (checkWrapper(toUnwrap, wrapper) == false) throw std::exception("Incorrectly wrapping string.\n");
	  typename std::basic_string<T>::const_iterator front = toUnwrap.begin(), last = toUnwrap.end() - 1;
	  if (wrapper == '\"' || wrapper == '/') {
		  // std::cout << toUnwrap << " unwrapPlus " << *front << " "  << *last << std::endl;
		  if (*front != *last) throw("Incorrectly wrapping string at: " + toUnwrap + "\n");
		  else return toUnwrap.substr(1, toUnwrap.length() - 2);
	  }
	  else {
		  if (wrapper == '(') {
			  if (*front == '(' && *last == ')') return toUnwrap.substr(1, toUnwrap.length() - 2);
			  else throw std::exception("Incorrectly wrapping string.\n");
		  }
		  else if (wrapper == '[') {
			  if (*front == '[' && *last == ']') return toUnwrap.substr(1, toUnwrap.length() - 2);
			  else throw std::exception("Incorrectly wrapping string.\n");
		  }
		  else if (wrapper == '{') {
			  if (*front == '{' && *last == '}') return toUnwrap.substr(1, toUnwrap.length() - 2);
			  else throw std::exception("Incorrectly wrapping string.\n");
		  }
		  else {
			  throw std::exception("Incorrectly wrapping string.\n");
		  }
	  }
  }

  // Check if string is correctly wrapped, also works when string wrapped by "()" or "[]"
  template<typename T>
  inline bool checkWrapper(const std::basic_string<T>& toCheck, T wrapper) {
	  if (toCheck.size() == 0) return false;
	  typename std::basic_string<T>::const_iterator front = toCheck.begin(), last = toCheck.end() - 1;
	  if (*front == wrapper && *last == wrapper) return true;
	  else if (*front == '(' && *last == ')' && wrapper == '(') return true;
	  else if (*front == '[' && *last == ']' && wrapper == '[') return true;
	  else return false;
  }

  /*--- split sentinel separated strings into a vector of trimmed strings ---*/

  template <typename T>
  inline std::vector<std::basic_string<T>> split(const std::basic_string<T>& toSplit, T splitOn = ',')
  {
    std::vector<std::basic_string<T>> splits;
    std::basic_string<T> temp;
    typename std::basic_string<T>::const_iterator iter;
    for (iter = toSplit.begin(); iter != toSplit.end(); ++iter)
    {
      if (*iter != splitOn)
      {
        temp += *iter;
      }
      else
      {
        splits.push_back(trim(temp));
        temp.clear();
      }
    }
    if (temp.length() > 0)
      splits.push_back(trim(temp));
    return splits;
  }

  // The modified version of split, which can skip splict character in the "[]" s
  // Split '[', ']', '\"' will be rejected.
  template <typename T>
  inline std::vector<std::basic_string<T>> splitPlus(const std::basic_string<T>& toSplit, T splitOn = ',')
  {
	  if (splitOn == '(' || splitOn == ')' || splitOn == '\"') throw std::exception("Split Plus: Unproper split of input string.\n");
	  std::vector<std::basic_string<T>> splits;
	  std::basic_string<T> temp;
	  typename std::basic_string<T>::const_iterator iter;
	  int exceptFlag = 0;
	  bool quoteFlag = false;
	  for (iter = toSplit.begin(); iter != toSplit.end(); ++iter)
	  {
		  if (*iter == '(')
		  {
			  exceptFlag += 1;
			  temp += *iter;
		  }
		  else if (*iter == ')')
		  {
			  exceptFlag -= 1;
			  temp += *iter;
		  }
		  else if (*iter == '\"')
		  {
			  quoteFlag = !quoteFlag;
			  temp += *iter;
		  }
		  else if (*iter != splitOn || (*iter == splitOn && (exceptFlag != 0 || quoteFlag == true)))
		  {
			  temp += *iter;
		  }
		  else
		  {
			  splits.push_back(trim(temp));
			  temp.clear();
		  }
	  }
	  if (temp.length() > 0)
		  splits.push_back(trim(temp));
	  return splits;
  }

  /*--- show collection of string splits ------------------------------------*/

  template <typename T>
  inline void showSplits(const std::vector<std::basic_string<T>>& splits, std::ostream& out = std::cout)
  {
    out << "\n";
    for (auto item : splits)
    {
      if (item == "\n")
        out << "\n--" << "newline";
      else
        out << "\n--" << item;
    }
    out << "\n";
  }

  // Escape all metacharacters in regular expressions, used in Project2.
  inline std::string regexSafeFilter(const std::string& toFilter) {
	  if (toFilter.length() == 0) return toFilter;
	  std::string result = "";
	  auto index = toFilter.begin();
	  while (index != toFilter.end()) {
		  if (*index == '^' || *index == '$' || *index == '*' || *index == '+' ||
			  *index == '?' || *index == '(' || *index == ')' || *index == '{' ||
			  *index == '}' || *index == '.' || *index == '[' || *index == ']') {
			  result = result + "\\" + (*index);
		  }
		  else {
			  result = result + (*index);
		  }
		  index++;
	  }
	  return result;
  }
}
#endif
