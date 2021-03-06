#include "vm.hpp"
#include "vm/object_utils.hpp"

#include "builtin/array.hpp"
#include "builtin/fixnum.hpp"
#include "builtin/tuple.hpp"
#include "builtin/class.hpp"
#include "objectmemory.hpp"
#include "primitives.hpp"

#include <iostream>

/* Implementation certain Array methods. These methods are just
 * the ones the VM requires, not the entire set of all Array methods.
 * This includes methods required to implement certain Array
 * primitives. */

namespace rubinius {

  void Array::init(STATE) {
    GO(array).set(state->new_class("Array", G(object), Array::fields));
    G(array)->set_object_type(state, ArrayType);
  }

  size_t Array::size() {
    return total_->to_native();
  }

  Array* Array::create(STATE, size_t idx) {
    Array* ary;
    ary = (Array*)state->om->new_object(G(array), Array::fields);

    ary->setup(state, idx);

    return ary;
  }

  Array* Array::from_tuple(STATE, Tuple* tup) {
    size_t length = tup->num_fields();
    Array* ary = Array::create(state, length);
    ary->tuple_->copy_from(state, tup,
			   Fixnum::from(0), Fixnum::from(length),
			   Fixnum::from(0));
    ary->total(state, Fixnum::from(length));
    return ary;
  }

  void Array::setup(STATE, size_t size) {
    this->tuple(state, Tuple::create(state, size));
    this->start(state, Fixnum::from(0));
    this->total(state, Fixnum::from(0));
  }

  // NOTE: We don't use Primitives::failure() here because the wrapper
  // code makes sure we're only called when the arity and type are correct.
  // Thus we know that this is a simple a[n] case only, which we can
  // fully handle.
  Object* Array::aref(STATE, Fixnum* idx) {
    native_int index = idx->to_native();
    const native_int start = start_->to_native();
    const native_int total = start + total_->to_native();

    // Handle negative indexes
    if(index < 0) {
      index += total;
    } else {
      index += start;
    }

    // Off either end, return nil
    if(index >= total || index < start) return Qnil;

    return tuple_->at(state, index);
  }

  Object* Array::aset(STATE, Fixnum* idx, Object* val) {
    native_int index = idx->to_native();

    if(index < 0) {
      return Primitives::failure();
    }

    return this->set(state, index, val);
  }

  Object* Array::get(STATE, size_t idx) {
    if(idx >= (size_t)total_->to_native()) {
      return Qnil;
    }

    idx += start_->to_native();

    return tuple_->at(state, idx);
  }

  Object* Array::set(STATE, size_t idx, Object* val) {
    size_t tuple_size = tuple_->num_fields();
    size_t oidx = idx;
    idx += start_->to_native();

    if(idx >= tuple_size) {
      // Uses the same algo as 1.8 to resize the tuple
      size_t new_size = tuple_size / 2;
      if(new_size < 3) {
        new_size = 3;
      }

      Tuple* nt = Tuple::create(state, new_size+idx);
      nt->copy_from(state, tuple_, start_, total_, Fixnum::from(0));
      tuple(state, nt);
      start(state, Fixnum::from(0));
      idx = oidx;
    }

    tuple_->put(state, idx, val);
    if((size_t)total_->to_native() <= oidx) {
      total(state, Fixnum::from(oidx+1));
    }
    return val;
  }

  void Array::unshift(STATE, Object* val) {
    size_t new_size = total_->to_native() + 1;
    size_t lend = start_->to_native();

    if(lend > 0) {
      tuple_->put(state, lend-1, val);
      start(state, Fixnum::from(lend-1));
      total(state, Fixnum::from(new_size));
    } else {
      Tuple* nt = Tuple::create(state, new_size);
      nt->copy_from(state, tuple_, start_, total_,
		    Fixnum::from(1));
      nt->put(state, 0, val);

      total(state, Fixnum::from(new_size));
      start(state, Fixnum::from(0));
      tuple(state, nt);
    }
  }

  Object* Array::shift(STATE) {
    size_t cnt = total_->to_native();

    if(cnt == 0) return Qnil;
    Object* obj = get(state, 0);
    set(state, 0, Qnil);
    start(state, Fixnum::from(start_->to_native() + 1));
    total(state, Fixnum::from(cnt - 1));
    return obj;
  }

  Object* Array::append(STATE, Object* val) {
    set(state, (size_t)total_->to_native(), val);
    return val;
  }

  bool Array::includes_p(STATE, Object* val) {
    size_t cnt = total_->to_native();

    for(size_t i = 0; i < cnt; i++) {
      if(get(state, i) == val) return true;
    }

    return false;
  }

  Object* Array::pop(STATE) {
    size_t cnt = total_->to_native();

    if(cnt == 0) return Qnil;
    Object *obj = get(state, cnt - 1);
    set(state, cnt-1, Qnil);
    total(state, Fixnum::from(cnt - 1));
    return obj;
  }

  void Array::Info::show(STATE, Object* self, int level) {
    Array* ary = as<Array>(self);
    size_t size = ary->size();
    size_t stop = size < 5 ? size : 5;

    if(size == 0) {
      class_info(state, self, true);
      return;
    }

    class_info(state, self);
    std::cout << ": " << size << std::endl;
    ++level;
    for(size_t i = 0; i < stop; i++) {
      indent(level);
      Object* obj = ary->get(state, i);
      if(obj == ary) {
        class_info(state, obj, true);
      } else {
        obj->show(state, level);
      }
    }
    if(ary->size() > stop) ellipsis(level);
    close_body(level);
  }
}
