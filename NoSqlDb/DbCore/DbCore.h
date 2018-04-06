#pragma once
/////////////////////////////////////////////////////////////////////
// DbCore.h - Implements NoSql database prototype                  //
// ver 1.0                                                         //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2018       //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides two classes:
* - DbCore implements core NoSql database operations, but does not
*   provide editing, querying, or persisting.  Those are left as
*   exercises for students.
* - DbElement provides the value part of our key-value database.
*   It contains fields for name, description, date, child collection
*   and a payload field of the template type. 
* The package also provides functions for displaying:
* - set of all database keys
* - database elements
* - all records in the database

* Required Files:
* ---------------
* DbCore.h, DbCore.cpp
* DateTime.h, DateTime.cpp
* Utilities.h, Utilities.cpp
*
* Maintenance History:
* --------------------
* ver 1.0 : 10 Jan 2018
*/

#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include "../DateTime/DateTime.h"

namespace NoSqlDb
{
	template<typename T, typename Children>
	class payLoadComplex {
	public:
		Children& category() { return category_; }
		Children category() const { return category_; }
		void category(const Children& category) { category_ = category; }

		T& payLoad() { return payLoad_; }
		T payLoad() const { return payLoad_; }
		void payLoad(const T& payLoad) { payLoad_ = payLoad; }

		std::string& nameSpace() { return nameSpace_; }
		std::string nameSpace() const { return nameSpace_; };
		void nameSpace(const std::string& nameSpace) { nameSpace_ = nameSpace; }

		std::string& status() { return status_; }
		std::string status() const { return status_; };
		void status(const std::string& status) { status_ = status; }

		std::string& owner() { return owner_; }
		std::string owner() const { return owner_; };
		void owner(const std::string& owner) { owner_ = owner; }

	private:
		Children category_;
		T payLoad_;
		std::string nameSpace_;
		std::string status_;
		std::string owner_;
	};
  /////////////////////////////////////////////////////////////////////
  // DbElement class
  // - provides the value part of a NoSql key-value database

  template<typename T>
  class DbElement
  {
  public:
    using Key = std::string;
    using Children = std::vector<Key>;
	using payLoadComplex = payLoadComplex<T, Children>;

    // methods to get and set DbElement fields

    std::string& name() { return name_; }
    std::string name() const { return name_; }
    void name(const std::string& name) { name_ = name; }

    std::string& descrip() { return descrip_; }
    std::string descrip() const { return descrip_; }
    void descrip(const std::string& name) { descrip_ = name; }
    
    DateTime& dateTime() { return dateTime_; }
    DateTime dateTime() const { return dateTime_; }
    void dateTime(const DateTime& dateTime) { dateTime_ = dateTime; }

    Children& children() { return children_; }
    const Children& children() const { return children_; }
    void children(const Children& children) { children_ = children; }

	Children& category() { return payLoads_.category(); }
	Children category() const { return payLoads_.category(); }
	void category(const Children& category) { payLoads_.category(category); }

    T& payLoad() { return payLoads_.payLoad(); }
    T payLoad() const { return payLoads_.payLoad(); }
    void payLoad(const T& payLoad) { payLoads_.payLoad(payLoad); }

	std::string& nameSpace() { return payLoads_.nameSpace(); }
	std::string nameSpace() const { return payLoads_.nameSpace(); };
	void nameSpace(const std::string& nameSpace) { payLoads_.nameSpace(nameSpace); }

	std::string& status() { return payLoads_.status(); }
	std::string status() const { return payLoads_.status(); };
	void status(const std::string& status) { payLoads_.status(status); }

	std::string& owner() { return payLoads_.owner(); }
	std::string owner() const { return payLoads_.owner(); };
	void owner(const std::string& owner) { payLoads_.owner(owner); }

	payLoadComplex& payLoads() { return payLoads_; }
	payLoadComplex payLoads() const { return payLoads_; }
	void payLoads(const payLoadComplex& another) {
		payLoads_.category(another.category());
		payLoads_.payLoad(another.payLoad());
		payLoads_.nameSpace(another.nameSpace());
		payLoads_.status(another.status());
		payLoads_.owner(another.owner());
	}

  private:
    std::string name_;
    std::string descrip_;
    DateTime dateTime_;
    Children children_;
	payLoadComplex payLoads_;
  };

  /////////////////////////////////////////////////////////////////////
  // DbCore class
  // - provides core NoSql db operations
  // - does not provide editing, querying, or persistance operations

  template <typename T>
  class DbCore
  {
  public:
    using Key = std::string;
    using Keys = std::vector<Key>;
    using Children = Keys;
    using DbStore = std::unordered_map<Key,DbElement<T>>;
    using iterator = typename DbStore::iterator;

    // methods to access database elements

    Keys keys();
    bool contains(const Key& key);
    size_t size();
    DbElement<T>& operator[](const Key& key);
    DbElement<T> operator[](const Key& key) const;
    typename iterator begin() { return dbStore_.begin(); }
    typename iterator end() { return dbStore_.end(); }

    // methods to get and set the private database hash-map storage

    DbStore& dbStore() { return dbStore_; }
    DbStore dbStore() const { return dbStore_; }
    void dbStore(const DbStore& dbStore) { dbStore_ = dbStore; }
  
  private:
    DbStore dbStore_;
  };

  /////////////////////////////////////////////////////////////////////
  // DbCore<T> methods

  //----< does db contain this key? >----------------------------------

  template<typename T>
  bool DbCore<T>::contains(const Key& key)
  {
    iterator iter = dbStore_.find(key);
    return iter != dbStore_.end();
  }
  //----< returns current key set for db >-----------------------------

  template<typename T>
  typename DbCore<T>::Keys DbCore<T>::keys()
  {
    DbCore<T>::Keys dbKeys;
    DbStore& dbs = dbStore();
    size_t size = dbs.size();
    dbKeys.reserve(size);
    for (auto item : dbs)
    {
      dbKeys.push_back(item.first);
    }
    return dbKeys;
  }
  //----< return number of db elements >-------------------------------

  template<typename T>
  size_t DbCore<T>::size()
  {
    return dbStore_.size();
  }
  //----< extracts value from db with key >----------------------------
  /*
  *  - indexes non-const db objects
  */
  template<typename T>
  DbElement<T>& DbCore<T>::operator[](const Key& key)
  {
    if (!contains(key))
    {
      dbStore_[key] = DbElement<T>();
    }
    return dbStore_[key];
  }
  //----< extracts value from db with key >----------------------------
  /*
  *  - indexes const db objects
  */
  template<typename T>
  DbElement<T> DbCore<T>::operator[](const Key& key) const
  {
    if (!contains(key))
    {
      dbStore_[key] = DbElement<T>();
    }
    return dbStore_[key];
  }
  
  /////////////////////////////////////////////////////////////////////
  // display functions

  //----< display database key set >-----------------------------------

  template<typename T>
  void showKeys(DbCore<T>& db, std::ostream& out = std::cout)
  {
    out << "\n  ";
    for (auto key : db.keys())
    {
      out << key << " ";
    }
  }
  //----< show record header items >-----------------------------------

  inline void showHeader(std::ostream& out = std::cout)
  {
    out << "\n  ";
    out << std::setw(26) << std::left << "DateTime";
    out << std::setw(10) << std::left << "Name";
    out << std::setw(25) << std::left << "Description";
	out << std::setw(25) << std::left << "Namespace";
	out << std::setw(25) << std::left << "Status";
	out << std::setw(25) << std::left << "Owner";
    out << std::setw(25) << std::left << "Payload";
    out << "\n  ";
    out << std::setw(26) << std::left << "------------------------";
    out << std::setw(10) << std::left << "--------";
    out << std::setw(25) << std::left << "-----------------------";
    out << std::setw(25) << std::left << "-----------------------";
	out << std::setw(25) << std::left << "-----------------------";
	out << std::setw(25) << std::left << "-----------------------";
	out << std::setw(25) << std::left << "-----------------------";
  }
  //----< display DbElements >-----------------------------------------

  template<typename T>
  void showElem(const DbElement<T>& el, std::ostream& out = std::cout)
  {
    out << "\n  ";
    out << std::setw(26) << std::left << std::string(el.dateTime());
    out << std::setw(10) << std::left << el.name();
    out << std::setw(25) << std::left << el.descrip();
    out << std::setw(25) << std::left << el.payLoad();
    typename DbElement<T>::Children children = el.children();
    if (children.size() > 0)
    {
      out << "\n    child keys: ";
      for (auto key : children)
      {
        out << " " << key;
      }
    }
  }
  //----< display all records in database >----------------------------

  template<typename T>
  void showDb(const DbCore<T>& db, std::ostream& out = std::cout)
  {
    //out << "\n  ";
    //out << std::setw(26) << std::left << "DateTime";
    //out << std::setw(10) << std::left << "Name";
    //out << std::setw(25) << std::left << "Description";
    //out << std::setw(25) << std::left << "Payload";
    //out << "\n  ";
    //out << std::setw(26) << std::left << "------------------------";
    //out << std::setw(10) << std::left << "--------";
    //out << std::setw(25) << std::left << "-----------------------";
    //out << std::setw(25) << std::left << "-----------------------";
    showHeader(out);
    typename DbCore<T>::DbStore dbs = db.dbStore();
    for (auto item : dbs)
    {
      showElem(item.second, out);
    }
  }
}
