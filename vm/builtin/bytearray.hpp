#ifndef RBX_BUILTIN_BYTEARRAY_HPP
#define RBX_BUILTIN_BYTEARRAY_HPP

#include "builtin/object.hpp"
#include "type_info.hpp"

#include <ctype.h>
#include "object_types.hpp"

namespace rubinius {
  class ByteArray : public Object {
    public:
    const static size_t fields = 0;
    const static object_type type = ByteArrayType;

    // Body access
    uint8_t bytes[];

    static void init(STATE);
    static ByteArray* create(STATE, size_t bytes);

    // Ruby.primitive :bytearray_allocate
    static ByteArray* allocate(STATE, Integer* bytes);

    // Ruby.primitive :bytearray_size
    Integer* size(STATE);

    // Return the number of bytes this ByteArray contains
    size_t size() {
      return SIZE_OF_BODY(this);
    }

    // Ruby.primitive :bytearray_get_byte
    Fixnum* get_byte(STATE, Integer* index);

    // Ruby.primitive :bytearray_set_byte
    Fixnum* set_byte(STATE, Integer* index, Fixnum* value);

    // Ruby.primitive :bytearray_move_bytes
    Integer* move_bytes(STATE, Integer* start, Integer* count, Integer* dest);

    // Ruby.primitive :bytearray_fetch_bytes
    ByteArray* fetch_bytes(STATE, Integer* start, Integer* count);

    // Ruby.primitive :bytearray_compare_bytes
    Fixnum* compare_bytes(STATE, ByteArray* other, Integer* a, Integer* b);

    // Ruby.primitive :bytearray_dup_into
    ByteArray* dup_into(STATE, ByteArray* other);

    /* ::locate searches for +pattern+ in the ByteArray. Returns the
     * number of characters from the front of the ByteArray to the end
     * of the pattern if a match is found. Returns Qnil if a match is
     * not found. Starts searching at index +start+.
     */

    // Ruby.primitive :bytearray_locate
    Object* locate(STATE, String* pattern, Integer* start);

    char* to_chars(STATE);

    class Info : public TypeInfo {
    public:
      Info(object_type type, bool cleanup = false): TypeInfo(type, cleanup) { }
      virtual void mark(Object* t, ObjectMark& mark);
    };
  };
};

#endif
