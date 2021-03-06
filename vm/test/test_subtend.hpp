#include <cxxtest/TestSuite.h>

/* System */
#include "subtend/PortableUContext.h"

/* Project */
#include "vm.hpp"
#include "message.hpp"
#include "builtin/array.hpp"
#include "builtin/module.hpp"
#include "builtin/fixnum.hpp"
#include "builtin/string.hpp"
#include "builtin/symbol.hpp"
#include "builtin/task.hpp"

/* Testee */
#include "builtin/nativemethod.hpp"
#include "builtin/nativemethodcontext.hpp"

#include "subtend/ruby.h"

/* Re-reset */
#undef  Qfalse
#undef  Qtrue
#undef  Qnil
#undef  Qundef

#define Qfalse RBX_Qfalse
#define Qtrue  RBX_Qtrue
#define Qnil   RBX_Qnil
#define Qundef RBX_Qundef

#define Sfalse ((VALUE)6L)
#define Strue  ((VALUE)10L)
#define Snil   ((VALUE)14L)
#define Sundef ((VALUE)18L)

/* From vm/objectmemory.hpp */
#undef  ALLOC_N
#define ALLOC_N(type, size) ((type*)calloc(size, sizeof(type)))

static NativeMethodContext* hidden_context = NULL;
static Array* hidden_ruby_array = NULL;
static Object* hidden_receiver = NULL;
static int hidden_argc = 0;
static Handle* hidden_c_array = NULL;

using namespace rubinius;

extern "C" {

  Handle minus_three_arity(Handle ruby_array)
  {
    hidden_context = NativeMethodContext::current();
    hidden_ruby_array = as<Array>(hidden_context->object_from(ruby_array));

    Object* first = hidden_ruby_array->get(hidden_context->state(), 0);

    return hidden_context->handle_for(first);
  }

  Handle minus_two_arity(Handle receiver, Handle ruby_array)
  {
    hidden_context = NativeMethodContext::current();
    hidden_receiver = hidden_context->object_from(receiver);
    hidden_ruby_array = as<Array>(hidden_context->object_from(ruby_array));

    Object* first = hidden_ruby_array->get(hidden_context->state(), 0);

    return hidden_context->handle_for(first);
  }

  Handle minus_one_arity(int argc, Handle* args, Handle receiver)
  {
    hidden_context = NativeMethodContext::current();
    hidden_argc = argc;
    hidden_c_array = args;
    hidden_receiver = hidden_context->object_from(receiver);

    return args[0];
  }

  Handle one_arg(Handle receiver, Handle arg1)
  {
    hidden_context = NativeMethodContext::current();
    hidden_receiver = hidden_context->object_from(receiver);

    return hidden_context->handle_for(Fixnum::from(1));
  }

  Handle two_arg(Handle receiver, Handle arg1, Handle arg2)
  {
    hidden_context = NativeMethodContext::current();
    hidden_receiver = hidden_context->object_from(receiver);

    return hidden_context->handle_for(Fixnum::from(2));
  }

  Handle three_arg(Handle receiver, Handle arg1, Handle arg2, Handle arg3)
  {
    hidden_context = NativeMethodContext::current();
    hidden_receiver = hidden_context->object_from(receiver);

    return hidden_context->handle_for(Fixnum::from(3));
  }

  Handle four_arg(Handle receiver, Handle arg1, Handle arg2, Handle arg3, Handle arg4)
  {
    hidden_context = NativeMethodContext::current();
    hidden_receiver = hidden_context->object_from(receiver);

    return hidden_context->handle_for(Fixnum::from(4));
  }

  Handle five_arg(Handle receiver, Handle arg1, Handle arg2, Handle arg3, Handle arg4, Handle arg5)
  {
    hidden_context = NativeMethodContext::current();
    hidden_receiver = hidden_context->object_from(receiver);

    return hidden_context->handle_for(Fixnum::from(5));
  }

  Handle funcaller2(Handle receiver, Handle array, Handle obj)
  {
    hidden_context = NativeMethodContext::current();
    Symbol* method = hidden_context->state()->symbol("blah");

    Handle args[1];

    args[0] = obj;

    /* Handle ret = */ rb_funcall2(array, reinterpret_cast<ID>(method), 1, args);

    return obj;
  }

}


class TestSubtend : public CxxTest::TestSuite
{
  public:

  VM* my_state;
  Task* my_task;
  Message* my_message;
  Module* my_module;

  void setUp() {
    my_state = new VM(65536 * 1000);
    my_task = Task::create(my_state);
    my_message = new Message(my_state);
    my_module = Module::create(my_state);

    my_message->set_caller(my_task->active());
  }

  void tearDown() {
    hidden_context = NULL;
    hidden_ruby_array = NULL;
    hidden_argc = 0;
    hidden_c_array = NULL;

    delete my_message;
    delete my_state;
  }

  /* Tests */

  void test_ruby_to_c_call_with_args_as_ruby_array()
  {
    Array* args = Array::create(my_state, 10);
    Array* control = Array::create(my_state, 10);

    for (std::size_t i = 0; i < 10; ++i) {
      Object* obj = my_state->new_object(my_state->globals.object.get());

      args->set(my_state, i, obj);
      control->set(my_state, i, obj);
    }

    my_message->set_arguments(my_state, args);
    my_message->name = my_state->symbol("__subtend_fake_test_method__");

    NativeMethod* method = NativeMethod::create(my_state,
                                                String::create(my_state, __FILE__),
                                                my_module,
                                                my_message->name,
                                                &minus_three_arity,
                                                Fixnum::from(ARGS_IN_RUBY_ARRAY));

    my_message->method = method;
    bool ret = method->execute(my_state, my_task, *my_message);

    TS_ASSERT_EQUALS(ret, true);

    TS_ASSERT_DIFFERS(hidden_context, static_cast<NativeMethodContext*>(NULL));

    TS_ASSERT_EQUALS(hidden_ruby_array, args);

    for (std::size_t i = 0; i < 10; ++i) {
      TS_ASSERT_EQUALS(hidden_ruby_array->get(my_state, i), control->get(my_state, i));
    }

    TS_ASSERT_EQUALS(hidden_context->return_value(), hidden_ruby_array->get(my_state, 0));
  }

  void test_ruby_to_c_call_with_receiver_and_args_as_ruby_array()
  {
    Array* args = Array::create(my_state, 10);
    Array* control = Array::create(my_state, 10);

    for (std::size_t i = 0; i < 10; ++i) {
      Object* obj = my_state->new_object(my_state->globals.object.get());

      args->set(my_state, i, obj);
      control->set(my_state, i, obj);
    }

    Object* receiver = my_state->new_object(my_state->globals.object.get());

    my_message->recv = receiver;
    my_message->set_arguments(my_state, args);
    my_message->name = my_state->symbol("__subtend_fake_test_method__");

    NativeMethod* method = NativeMethod::create(my_state,
                                                String::create(my_state, __FILE__),
                                                my_module,
                                                my_message->name,
                                                &minus_two_arity,
                                                Fixnum::from(RECEIVER_PLUS_ARGS_IN_RUBY_ARRAY));

    my_message->method = method;
    bool ret = method->execute(my_state, my_task, *my_message);

    TS_ASSERT_EQUALS(ret, true);

    TS_ASSERT_DIFFERS(hidden_context, static_cast<NativeMethodContext*>(NULL));

    TS_ASSERT_EQUALS(hidden_receiver, receiver);
    TS_ASSERT_EQUALS(hidden_ruby_array, args);

    for (std::size_t i = 0; i < 10; ++i) {
      TS_ASSERT_EQUALS(hidden_ruby_array->get(my_state, i), control->get(my_state, i));
    }

    TS_ASSERT_EQUALS(hidden_context->return_value(), hidden_ruby_array->get(my_state, 0));
  }

  void test_ruby_to_c_call_with_arg_count_args_in_c_array_plus_receiver()
  {
    Array* args = Array::create(my_state, 10);
    Array* control = Array::create(my_state, 10);

    for (std::size_t i = 0; i < 10; ++i) {
      Object* obj = my_state->new_object(my_state->globals.object.get());

      args->set(my_state, i, obj);
      control->set(my_state, i, obj);
    }

    Object* receiver = my_state->new_object(my_state->globals.object.get());

    my_message->recv = receiver;
    my_message->set_arguments(my_state, args);
    my_message->name = my_state->symbol("__subtend_fake_test_method__");

    NativeMethod* method = NativeMethod::create(my_state,
                                                String::create(my_state, __FILE__),
                                                my_module,
                                                my_message->name,
                                                &minus_one_arity,
                                                Fixnum::from(ARG_COUNT_ARGS_IN_C_ARRAY_PLUS_RECEIVER));

    my_message->method = method;
    bool ret = method->execute(my_state, my_task, *my_message);

    TS_ASSERT_EQUALS(ret, true);

    TS_ASSERT_DIFFERS(hidden_context, static_cast<NativeMethodContext*>(NULL));

    TS_ASSERT_EQUALS(hidden_receiver, receiver);
    TS_ASSERT_EQUALS(hidden_argc, 10);

    for (std::size_t i = 0; i < 10; ++i) {
      TS_ASSERT_EQUALS(hidden_context->object_from(hidden_c_array[i]), control->get(my_state, i));
    }

    TS_ASSERT_EQUALS(hidden_context->return_value(), control->get(my_state, 0));
  }

  /* TODO: Should check object identities too? */
  void test_ruby_to_c_call_with_recv_plus_one()
  {
    int arg_count = 1;

    Array* args = Array::create(my_state, arg_count);
    Array* control = Array::create(my_state, arg_count);

    for (int i = 0; i < arg_count; ++i) {
      Object* obj = my_state->new_object(my_state->globals.object.get());

      args->set(my_state, i, obj);
      control->set(my_state, i, obj);
    }

    Object* receiver = my_state->new_object(my_state->globals.object.get());

    my_message->recv = receiver;
    my_message->set_arguments(my_state, args);
    my_message->name = my_state->symbol("__subtend_fake_test_method__");

    NativeMethod* method = NativeMethod::create(my_state,
                                                String::create(my_state, __FILE__),
                                                my_module,
                                                my_message->name,
                                                &one_arg,
                                                Fixnum::from(arg_count));

    my_message->method = method;
    method->execute(my_state, my_task, *my_message);

    TS_ASSERT_EQUALS(hidden_receiver, receiver);
    TS_ASSERT_EQUALS(as<Fixnum>(hidden_context->return_value())->to_int(), arg_count);
  }

  void test_ruby_to_c_call_clears_caller_stack()
  {
    Task* task = Task::create(my_state, 2);
    task->push(Fixnum::from(4));

    MethodContext* top = task->active();

    TS_ASSERT_EQUALS(top->calculate_sp(), 0);

    Object* receiver = my_state->new_object(my_state->globals.object.get());

    my_message->recv = receiver;
    my_message->use_from_task(task, 1);
    my_message->name = my_state->symbol("__subtend_fake_test_method__");
    my_message->stack = 1;

    NativeMethod* method = NativeMethod::create(my_state,
                                                String::create(my_state, __FILE__),
                                                my_module,
                                                my_message->name,
                                                &one_arg,
                                                Fixnum::from(1));

    my_message->method = method;
    method->execute(my_state, task, *my_message);

    TS_ASSERT_EQUALS(task->active(), top);
    TS_ASSERT_EQUALS(top->calculate_sp(), 0);
    TS_ASSERT_EQUALS(hidden_receiver, receiver);
    TS_ASSERT_EQUALS(as<Fixnum>(hidden_context->return_value())->to_int(), 1);
    TS_ASSERT_EQUALS(as<Fixnum>(top->pop())->to_int(), 1);
  }

  void test_ruby_to_c_call_with_recv_plus_two()
  {
    int arg_count = 2;

    Array* args = Array::create(my_state, arg_count);
    Array* control = Array::create(my_state, arg_count);

    for (int i = 0; i < arg_count; ++i) {
      Object* obj = my_state->new_object(my_state->globals.object.get());

      args->set(my_state, i, obj);
      control->set(my_state, i, obj);
    }

    Object* receiver = my_state->new_object(my_state->globals.object.get());

    my_message->recv = receiver;
    my_message->set_arguments(my_state, args);
    my_message->name = my_state->symbol("__subtend_fake_test_method__");

    NativeMethod* method = NativeMethod::create(my_state,
                                                String::create(my_state, __FILE__),
                                                my_module,
                                                my_message->name,
                                                &two_arg,
                                                Fixnum::from(arg_count));

    my_message->method = method;
    method->execute(my_state, my_task, *my_message);

    TS_ASSERT_EQUALS(hidden_receiver, receiver);
    TS_ASSERT_EQUALS(as<Fixnum>(hidden_context->return_value())->to_int(), arg_count);
  }

  void test_ruby_to_c_call_with_recv_plus_three()
  {
    int arg_count = 3;

    Array* args = Array::create(my_state, arg_count);
    Array* control = Array::create(my_state, arg_count);

    for (int i = 0; i < arg_count; ++i) {
      Object* obj = my_state->new_object(my_state->globals.object.get());

      args->set(my_state, i, obj);
      control->set(my_state, i, obj);
    }

    Object* receiver = my_state->new_object(my_state->globals.object.get());

    my_message->recv = receiver;
    my_message->set_arguments(my_state, args);
    my_message->name = my_state->symbol("__subtend_fake_test_method__");

    NativeMethod* method = NativeMethod::create(my_state,
                                                String::create(my_state, __FILE__),
                                                my_module,
                                                my_message->name,
                                                &three_arg,
                                                Fixnum::from(arg_count));

    my_message->method = method;
    method->execute(my_state, my_task, *my_message);

    TS_ASSERT_EQUALS(hidden_receiver, receiver);
    TS_ASSERT_EQUALS(as<Fixnum>(hidden_context->return_value())->to_int(), arg_count);
  }

  void test_ruby_to_c_call_with_recv_plus_four()
  {
    int arg_count = 4;

    Array* args = Array::create(my_state, arg_count);
    Array* control = Array::create(my_state, arg_count);

    for (int i = 0; i < arg_count; ++i) {
      Object* obj = my_state->new_object(my_state->globals.object.get());

      args->set(my_state, i, obj);
      control->set(my_state, i, obj);
    }

    Object* receiver = my_state->new_object(my_state->globals.object.get());

    my_message->recv = receiver;
    my_message->set_arguments(my_state, args);
    my_message->name = my_state->symbol("__subtend_fake_test_method__");

    NativeMethod* method = NativeMethod::create(my_state,
                                                String::create(my_state, __FILE__),
                                                my_module,
                                                my_message->name,
                                                &four_arg,
                                                Fixnum::from(arg_count));

    my_message->method = method;
    method->execute(my_state, my_task, *my_message);

    TS_ASSERT_EQUALS(hidden_receiver, receiver);
    TS_ASSERT_EQUALS(as<Fixnum>(hidden_context->return_value())->to_int(), arg_count);
  }

  void test_ruby_to_c_call_with_recv_plus_five()
  {
    int arg_count = 5;

    Array* args = Array::create(my_state, arg_count);
    Array* control = Array::create(my_state, arg_count);

    for (int i = 0; i < arg_count; ++i) {
      Object* obj = my_state->new_object(my_state->globals.object.get());

      args->set(my_state, i, obj);
      control->set(my_state, i, obj);
    }

    Object* receiver = my_state->new_object(my_state->globals.object.get());

    my_message->recv = receiver;
    my_message->set_arguments(my_state, args);
    my_message->name = my_state->symbol("__subtend_fake_test_method__");

    NativeMethod* method = NativeMethod::create(my_state,
                                                String::create(my_state, __FILE__),
                                                my_module,
                                                my_message->name,
                                                &five_arg,
                                                Fixnum::from(arg_count));

    my_message->method = method;
    method->execute(my_state, my_task, *my_message);

    TS_ASSERT_EQUALS(hidden_receiver, receiver);
    TS_ASSERT_EQUALS(as<Fixnum>(hidden_context->return_value())->to_int(), arg_count);
  }

  void test_rb_funcall()
  {
    /* No actual Ruby code involved.. */
    CompiledMethod* target = CompiledMethod::create(my_state);
    target->iseq(my_state, InstructionSequence::create(my_state, 1));
    target->iseq()->opcodes()->put(my_state, 0, Fixnum::from(InstructionSequence::insn_ret));
    target->total_args(my_state, Fixnum::from(1));
    target->required_args(my_state, target->total_args());
    target->stack_size(my_state, Fixnum::from(1));

    Symbol* name = my_state->symbol("blah");
    my_state->globals.true_class.get()->method_table()->store(my_state, name, target);

    target->formalize(my_state);

    Array* args = Array::create(my_state, 2);
    Object* obj = my_state->new_object(my_state->globals.object.get());

    args->set(my_state, 0, Qtrue);
    args->set(my_state, 1, obj);

    Object* receiver = my_state->new_object(my_state->globals.object.get());

    my_message->recv = receiver;
    my_message->set_arguments(my_state, args);
    my_message->name = my_state->symbol("__subtend_fake_funcall_test_method__");

    NativeMethod* method = NativeMethod::create(my_state,
                                                String::create(my_state, __FILE__),
                                                my_module,
                                                my_message->name,
                                                &funcaller2,
                                                Fixnum::from(2));

    my_message->method = method;
    /* This only loads the cm. */
    method->execute(my_state, my_task, *my_message);

    my_task->active()->vmm->resume(my_task, my_task->active());

    TS_ASSERT_EQUALS(obj, my_task->active()->top());

    /* TODO: Need some more reasonable tests here? */
  }

};
