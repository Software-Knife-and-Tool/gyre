/********
 **
 **  SPDX-License-Identifier: MIT
 **
 **  Copyright (c) 2017-2022 James M. Putnam <putnamjm.design@gmail.com>
 **
 **/

/********
 **
 **  mu-type.cc: library type functions
 **
 **/
#include <cassert>
#include <functional>
#include <iomanip>
#include <map>
#include <vector>

#include "libmu/core.h"
#include "libmu/type.h"

#include "libmu/types/condition.h"
#include "libmu/types/cons.h"
#include "libmu/types/fixnum.h"
#include "libmu/types/float.h"
#include "libmu/types/function.h"
#include "libmu/types/stream.h"
#include "libmu/types/struct.h"
#include "libmu/types/symbol.h"

namespace libmu {
namespace mu {

using Condition = core::Condition;
using Frame = core::Env::Frame;
using Type = core::Type;

/** * (type-of object) => symbol **/
auto TypeOf(Frame* fp) -> void {
  auto obj = fp->argv[0];

  fp->value = core::Struct::IsType(obj)
                  ? core::Struct::stype(obj)
                  : Type::MapClassSymbol(Type::TypeOf(obj));
}

/** * (eq object object) => bool **/
auto Eq(Frame* fp) -> void {
  fp->value = Type::Bool(Type::Eq(fp->argv[0], fp->argv[1]));
}

/** * (special-operatorp form) => bool **/
auto IsSpecOp(Frame* fp) -> void {
  auto symbol = fp->argv[0];

  if (!core::Symbol::IsType(symbol))
    Condition::Raise(fp->env, Condition::CONDITION_CLASS::TYPE_ERROR,
                     "special-operatorp", symbol);

  fp->value = Type::Bool(core::IsSpecOp(symbol));
}

/** * (make-view object) => vector **/
auto MakeView(Frame* fp) -> void {
  fp->value = core::Env::ViewOf(fp->env, fp->argv[0]);
}

} /* namespace mu */
} /* namespace libmu */
