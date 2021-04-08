/********
 **
 **  SPDX-License-Identifier: MIT
 **
 **  Copyright (c) 2017-2022 James M. Putnam <putnamjm.design@gmail.com>
 **
 **/

/********
 **
 **  function.cc: library functions
 **
 **/
#include "libmu/types/function.h"

#include <cassert>
#include <sstream>

#include "libmu/core.h"
#include "libmu/env.h"
#include "libmu/type.h"

#include "libmu/compiler.h"

#include "libmu/types/condition.h"
#include "libmu/types/cons.h"
#include "libmu/types/fixnum.h"
#include "libmu/types/symbol.h"
#include "libmu/types/vector.h"

namespace libmu {
namespace core {
namespace {

/** * call function on frame **/
auto CallFrame(Env::Frame* fp) -> void {
  fp->value = Type::NIL;
  if (Type::Null(Function::mu(fp->func))) {
    Cons::cons_iter<Tag> iter(Cons::cdr(Function::form(fp->func)));
    for (auto it = iter.begin(); it != iter.end(); it = ++iter)
      fp->value = core::Eval(fp->env, it->car);
  } else
    Type::Untag<Env::TagFn>(Function::mu(fp->func))->fn(fp);
}

/** * arity checking **/
constexpr auto arity_nreqs(size_t arity) -> size_t { return arity >> 1; }
constexpr auto arity_rest(size_t arity) -> bool { return arity & 1; }

auto arity_nreqs(Tag fn) -> size_t {
  assert(Function::IsType(fn));

  return arity_nreqs(Function::arity(fn));
}

auto arity_rest(Tag fn) -> bool {
  assert(Function::IsType(fn));

  return arity_rest(Function::arity(fn));
}

/** * run-time function call argument arity validation **/
auto CheckArity(Env* env, Tag fn, const std::vector<Tag>& args) -> void {
  assert(Function::IsType(fn));

  size_t nreqs = arity_nreqs(fn);
  auto rest = arity_rest(fn);
  auto nargs = args.size();

  if (nargs < nreqs)
    Condition::Raise(env, Condition::CONDITION_CLASS::TYPE_ERROR,
                     "argument list arity: nargs (" + std::to_string(nargs) +
                         ") < nreqs (" + std::to_string(nreqs) + ") (funcall)",
                     fn);

  if (!rest && (nargs > nreqs))
    Condition::Raise(env, Condition::CONDITION_CLASS::TYPE_ERROR,
                     "argument list arity: !rest && nargs (" +
                         std::to_string(nargs) + ") > nreq (" +
                         std::to_string(nreqs) + ") (funcall)",
                     fn);
}

} /* anonymous namespace */

/** * garbage collection **/
auto Function::GcMark(Env* ev, Tag fn) -> void {
  assert(IsType(fn));

  if (!TagFormat<Layout>::IsGcMarked(fn)) {
    TagFormat<Layout>::GcMark(fn);
    ev->GcMark(ev, env(fn));
    ev->GcMark(ev, form(fn));
    ev->GcMark(ev, name(fn));
    for (auto fp : context(fn)) {
      Env::GcFrame(fp);
    }
  }
}

/** * function printer **/
auto Function::Print(Env* env, Tag fn, Tag str, bool) -> void {
  assert(IsType(fn));
  assert(Stream::IsType(str));

  auto stream = Stream::StreamDesignator(env, str);

  auto type =
      String::StdStringOf(Symbol::name(Type::MapClassSymbol(Type::TypeOf(fn))));

  auto name = Null(Function::name(fn))
                  ? std::string{":lambda"}
                  : String::StdStringOf(Symbol::name(Function::name(fn)));

  std::stringstream hexs;

  hexs << std::hex << Type::to_underlying(fn);
  core::PrintStdString(env,
                       "#<:" + type + " #x" + hexs.str() + " (" + name + ")>",
                       stream, false);
}

/** * make view of function **/
auto Function::ViewOf(Env* env, Tag fn) -> Tag {
  assert(IsType(fn));

  /* think: add context */
  auto view = std::vector<Tag>{Symbol::Keyword("func"),
                               fn,
                               Fixnum(ToUint64(fn) >> 3).tag_,
                               name(fn),
                               mu(fn),
                               form(fn),
                               frame_id(fn),
                               Fixnum(arity(fn)).tag_};

  return Vector(env, view).tag_;
}

/** * call function with argument vector **/
auto Function::Funcall(Env* env, Tag fn, const std::vector<Tag>& argv) -> Tag {
  assert(IsType(fn));

  size_t nargs = arity_nreqs(fn) + ((arity_rest(fn) ? 1 : 0));

  CheckArity(env, fn, argv);

  auto args = std::make_unique<Tag[]>(nargs);
  if (nargs) {
    size_t i = 0;
    for (; i < arity_nreqs(fn); i++) args[i] = argv[i];

    if (arity_rest(fn)) {
      if (i == argv.size()) {
        args[i] = NIL;
      } else {
        std::vector<Tag> restv;

        for (size_t j = i; j < argv.size(); j++) restv.push_back(argv[j]);

        args[i] = Cons::List(env, restv);
      }
    }
  }

  Env::Frame fp(env, frame_id(fn), fn, args.get(), nargs);

  env->PushFrame(&fp);

  if (nargs) env->Cache(&fp);

  for (auto frame : Function::context(fn))
    if (frame->nargs) env->Cache(frame);

  CallFrame(&fp);

  for (auto frame : Function::context(fn))
    if (frame->nargs) env->UnCache(frame);

  if (nargs) env->UnCache(&fp);

  env->PopFrame();

  return fp.value;
}

} /* namespace core */
} /* namespace libmu */
