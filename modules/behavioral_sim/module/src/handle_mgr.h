#ifndef _BM_HANDLE_MGR_H_
#define _BM_HANDLE_MGR_H_

#include <type_traits>

#include <Judy.h>

class HandleMgrIterator;
class constHandleMgrIterator;

class HandleMgr {
  // friend class HandleMgrIterator;
  // friend class constHandleMgrIterator;

public:

  // iterators: since they are very simple, I did not mind duplicating the
  // code. Maybe it would be a good idea to condition the const'ness of the
  // iterator with a boolean template, to unify the 2 iterators. See :
  // www.sj-vs.net/c-implementing-const_iterator-and-non-const-iterator-without-code-duplication/

  class const_iterator;

  class iterator
  {
    friend class const_iterator;
  
  public:
    iterator(HandleMgr *handle_mgr, unsigned index)
      : handle_mgr(handle_mgr), index(index) {}
    
    unsigned &operator*() {return index;}
    unsigned *operator->() {return &index;}
    
    bool operator==(const iterator &other) const {
      return (handle_mgr == other.handle_mgr) && (index == other.index);
    }
    
    bool operator!=(const iterator &other) const {
      return !(*this == other);
    }
    
    iterator& operator++() {
      int Rc_int;
      Word_t jindex = index;;
      J1N(Rc_int, handle_mgr->handles, jindex);
      index = jindex;
      return *this;
    }

    iterator operator++(int){
      // Use operator++()
      const iterator old(*this);
      ++(*this);
      return old;
    }

  private:
    HandleMgr *handle_mgr;
    unsigned index;
  };

  class const_iterator
  {
  public:
    const_iterator(const HandleMgr *handle_mgr, unsigned index)
      : handle_mgr(handle_mgr), index(index) {}

    const_iterator(const iterator &other)
      : handle_mgr(other.handle_mgr), index(other.index) {}
    
    const unsigned &operator*() const {return index;}
    const unsigned *operator->() const {return &index;}

    const_iterator& operator=(const const_iterator &other) {
      handle_mgr = other.handle_mgr;
      index = other.index;
      return *this;
    }

    bool operator==(const const_iterator &other) const {
      return (handle_mgr == other.handle_mgr) && (index == other.index);
    }
  
    bool operator!=(const const_iterator &other) const {
      return !(*this == other);
    }

    const const_iterator& operator++(){
      int Rc_int;
      Word_t jindex = index;
      J1N(Rc_int, handle_mgr->handles, jindex);
      index = jindex;
      return *this;
    }

    const const_iterator operator++(int){
      // Use operator++()
      const const_iterator old(*this);
      ++(*this);
      return old;
    }

  private:
    const HandleMgr *handle_mgr;
    unsigned index;
  };


public:
  HandleMgr()
    : handles((Pvoid_t) NULL) {}

  /* Copy constructor */
  HandleMgr(const HandleMgr& other)
    : handles((Pvoid_t) NULL) {
    int Rc_iter, Rc_set;
    Word_t index = 0;
    J1F(Rc_iter, other.handles, index);
    while(Rc_iter) {
      J1S(Rc_set, handles, index);
      J1N(Rc_iter, other.handles, index);
    }
  }

  /* Move constructor */
  HandleMgr(HandleMgr&& other) noexcept
  : handles(other.handles) {}

  ~HandleMgr() {
    Word_t bytes_freed;
    J1FA(bytes_freed, handles);
  }

  /* Copy assignment operator */
  HandleMgr &operator=(const HandleMgr &other) {    
    HandleMgr tmp(other); // re-use copy-constructor
    *this = std::move(tmp); // re-use move-assignment
    return *this;
  }
 
  /* Move assignment operator */
  HandleMgr &operator=(HandleMgr &&other) noexcept {
    // simplified move-constructor that also protects against move-to-self.
    std::swap(handles, other.handles); // repeat for all elements
    return *this;
  }

  bool operator==(const HandleMgr &other) const {
    return (handles == other.handles);
  }

  bool operator!=(const HandleMgr &other) const {
    return !(*this == other);
  }

  /* Return 0 on success, -1 on failure */

  int get_handle(unsigned *handle) {
    Word_t jhandle = 0;
    int Rc;

    J1FE(Rc, handles, jhandle); // Judy1FirstEmpty()
    if(!Rc) return -1;
    J1S(Rc, handles, jhandle); // Judy1Set()
    if(!Rc) return -1;

    *handle = jhandle;

    return 0;
  }

  int release_handle(unsigned handle) {
    int Rc;
    J1U(Rc, handles, handle); // Judy1Unset()
    return Rc ? 0 : -1;
  }

  size_t size() const {
    Word_t size;
    J1C(size, handles, 0, -1);
    return size;
  }

  bool valid_handle(unsigned handle) const {
    int Rc;
    J1T(Rc, handles, handle); // Judy1Test()
    return (Rc == 1);
  }

  // iterators

  iterator begin() {
    Word_t index = 0;
    int Rc_int;
    J1F(Rc_int, handles, index);
    return iterator(this, index);
  }

  const_iterator begin() const {
    Word_t index = 0;
    int Rc_int;
    J1F(Rc_int, handles, index);
    return const_iterator(this, index);
  }

  iterator end() {
    Word_t index = -1;
    int Rc_int;
    J1L(Rc_int, handles, index);
    return iterator(this, index);
  }

  const_iterator end() const {
    Word_t index = -1;
    int Rc_int;
    J1L(Rc_int, handles, index);
    return const_iterator(this, index);
  }

private:
  Pvoid_t handles;
};




#endif