/********
 **
 **  SPDX-License-Identifier: MIT
 **
 **  Copyright (c) 2017-2022 James M. Putnam <putnamjm.design@gmail.com>
 **
 **/

/********
 **
 **  vector-types.cc: library specialized vectors
 **
 **/
#include "libmu/types/vector.h"

#include <cassert>

#include "libmu/core.h"
#include "libmu/env.h"
#include "libmu/type.h"

#include "libmu/types/char.h"
#include "libmu/types/condition.h"
#include "libmu/types/cons.h"
#include "libmu/types/float.h"
#include "libmu/types/function.h"
#include "libmu/types/stream.h"

namespace libmu {
namespace core {
namespace {

template <typename T>
static auto VMap(Env* env, Tag func, Tag vector) -> Tag {
  assert(Function::IsType(func));
  assert(Vector<Tag>::IsType(vector));

  std::vector<T> vlist;
  Vector<T>::vector_iter iter(vector);
  for (auto it = iter.begin(); it != iter.end(); it = ++iter)
    vlist.push_back(T::VSpecOf(
        Function::Funcall(env, func, std::vector<Tag>{T(*it).tag_})));

  return Vector<T>(vlist).tag_;
}

template <typename T>
static auto VMapC(Env* env, Tag func, Tag vector) -> void {
  assert(Function::IsType(func));
  assert(Vector<Tag>::IsType(vector));

  Vector<T>::vector_iter iter(vector);
  for (auto it = iter.begin(); it != iter.end(); it = ++iter)
    (void)Function::Funcall(env, func, std::vector<Tag>{T(*it).tag_});
}

template <typename T>
static auto VList(Env* env, const std::function<bool(Tag)>& isType, Tag list)
    -> Tag {
  std::vector<T> vec;

  Cons::cons_iter<Tag> iter(list);
  for (auto it = iter.begin(); it != iter.end(); it = ++iter) {
    auto form = it->car;
    if (!isType(form))
      Condition::Raise(env, Condition::CONDITION_CLASS::TYPE_ERROR,
                       "type mismatch in vector initialization", form);
    vec.push_back(T::VSpecOf(form));
  }

  return Vector<T>(vec).tag_;
}

} /* anonymous namespace */

/** * map a function onto a vector **/
auto Vector<Tag>::Map(Env* env, Tag func, Tag vector) -> Tag {
  assert(Function::IsType(func));
  assert(Vector<Tag>::IsType(vector));

  switch (Vector::TypeOf(vector)) {
    case SYS_CLASS::T:
      return VMap<Tag>(env, func, vector);
    case SYS_CLASS::FLOAT:
      return VMap<float>(env, func, vector);
    case SYS_CLASS::FIXNUM:
      return VMap<int64_t>(env, func, vector);
    case SYS_CLASS::CHAR:
      return VMap<char>(env, func, vector);
    case SYS_CLASS::BYTE:
      return VMap<uint8_t>(env, func, vector);
    default:
      assert(!"vector type botch");
  }
}

/** * map a function over a vector **/
auto Vector<Tag>::MapC(Env* env, Tag func, Tag vector) -> void {
  assert(Function::IsType(func));
  assert(Vector::IsType(vector));

  switch (Vector::TypeOf(vector)) {
    case SYS_CLASS::T:
      return VMapC<Tag>(env, func, vector);
    case SYS_CLASS::FLOAT:
      return VMapC<float>(env, func, vector);
    case SYS_CLASS::FIXNUM:
      return VMapC<uint64_t>(env, func, vector);
    case SYS_CLASS::CHAR:
      return VMapC<char>(env, func, vector);
    case SYS_CLASS::BYTE:
      return VMapC<uint8_t>(env, func, vector);
    default:
      assert(!"vector type botch");
  }
}

/** * list to vector **/
auto Vector<Tag>::ListToVector(Env* env, Tag vectype, Tag list) -> Tag {
  assert(Cons::IsList(list));

  auto vtype = Type::MapSymbolClass(vectype);

  switch (vtype) {
    case SYS_CLASS::T:
      return VList<Tag>(
          env, [](Tag) { return true; }, list);
    case SYS_CLASS::FLOAT:
      return VList<float>(env, Float::IsType, list);
    case SYS_CLASS::FIXNUM:
      return VList<int64_t>(env, Fixnum::IsType, list);
    case SYS_CLASS::BYTE:
      return VList<uint8_t>(env, Fixnum::IsType, list);
    case SYS_CLASS::CHAR:
      return VList<char>(env, Char::IsType, list);
    default:
      assert(!"vector type botch");
  }
}

/** * print vector to stream **/
template <>
auto Vector::Print(Env* env, Tag vector, Tag stream, bool esc) -> void {
  assert(IsType(vector));
  assert(Stream::IsType(stream));

  switch (Vector::TypeOf(vector)) {
    case SYS_CLASS::BYTE: {
      core::PrintStdString(env, "#(:byte", stream, false);

      vector_iter iter(vector);
      for (auto it = iter.begin(); it != iter.end(); it = ++iter) {
        core::PrintStdString(env, " ", stream, false);
        core::Print(env, Fixnum(*iter).tag_, stream, esc);
      }

      core::PrintStdString(env, ")", stream, false);
      break;
    }
    case SYS_CLASS::CHAR: {
      if (esc) core::PrintStdString(env, "\"", stream, false);

      vector_iter iter(vector);
      for (auto it = iter.begin(); it != iter.end(); it = ++iter)
        core::Print(env, Char(*it).tag_, stream, false);

      if (esc) core::PrintStdString(env, "\"", stream, false);
      break;
    }
    case SYS_CLASS::FIXNUM: {
      core::PrintStdString(env, "#(:fixnum", stream, false);

      vector_iter iter(vector);
      for (auto it = iter.begin(); it != iter.end(); it = ++iter) {
        core::PrintStdString(env, " ", stream, false);
        core::Print(env, Fixnum(*iter).tag_, stream, esc);
      }

      core::PrintStdString(env, ")", stream, false);
      break;
    }
    case SYS_CLASS::FLOAT: {
      core::PrintStdString(env, "#(:float", stream, false);

      vector_iter iter(vector);
      for (auto it = iter.begin(); it != iter.end(); it = ++iter) {
        core::PrintStdString(env, " ", stream, false);
        core::Print(env, Float(*iter).tag_, stream, esc);
      }

      core::PrintStdString(env, ")", stream, false);
      break;
    }
    case SYS_CLASS::T: {
      core::PrintStdString(env, "#(:t", stream, false);

      vector_iter iter(vector);
      for (auto it = iter.begin(); it != iter.end(); it = ++iter) {
        core::PrintStdString(env, " ", stream, false);
        core::Print(env, *iter, stream, esc);
      }

      core::PrintStdString(env, ")", stream, false);
      break;
    }
    default:
      assert(!"vector type botch");
      break;
  }
}

} /* namespace core */
} /* namespace libmu */
