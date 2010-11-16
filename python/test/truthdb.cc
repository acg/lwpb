#include "generated/test.pb.h"
#include "python_format.h"
#include <google/protobuf/stubs/strutil.h>
#include <iostream>
#include <sstream>

using namespace std;
using namespace google::protobuf;


/* Declare a named test case generator */

typedef Message* (*test_generator_t)();

typedef struct {
  const char* name;
  test_generator_t fn;
} test_case_t;


/* Test case generators */

Message* test_double()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  m->set_f_double(11.0 / 7);
  return m;
}

Message* test_zero_int32()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  m->set_f_int32(0);
  return m;
}

Message* test_positive_int32()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  m->set_f_int32(267914296); // fib(42)
  return m;
}

Message* test_negative_int32()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  m->set_f_int32(-267914296);
  return m;
}

Message* test_zero_sint32()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  m->set_f_sint32(0);
  return m;
}

Message* test_positive_sint32()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  m->set_f_sint32(267914296);
  return m;
}

Message* test_negative_sint32()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  m->set_f_sint32(-267914296);
  return m;
}

Message* test_fixed32()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  m->set_f_fixed32(267914296);
  return m;
}

Message* test_fixed64()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  uint64_t x = 0xc2c2c2c2;
  x <<= 32;
  x |= 0xc2c2c2c2;
  m->set_f_uint64(x);
  return m;
}

Message* test_max_uint32()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  m->set_f_uint32(0xffffffff);
  return m;
}

Message* test_max_uint64()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  uint64_t x = 0xffffffff;
  x <<= 32;
  x |= 0xffffffff;
  m->set_f_uint64(x);
  return m;
}

Message* test_true_bool()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  m->set_f_bool(true);
  return m;
}

Message* test_false_bool()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  m->set_f_bool(false);
  return m;
}

Message* test_string()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  m->set_f_string("hello world");
  return m;
}

Message* test_empty_string()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  m->set_f_string("");
  return m;
}

Message* test_nulls_string()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  m->set_f_string("hello\0world\0!");
  return m;
}

Message* test_utf8_string()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  m->set_f_string("\xc2\xab Copyright \xc2\xa9 1984 \xc2\xbb");
  return m;
}

Message* test_long_string()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;

  unsigned int i;
  string s;

  for (i=0; i<26; i++)
    s += string(1024, 'a'+i);

  m->set_f_string(s);

  return m;
}

Message* test_bytes()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;

  unsigned int i;
  string s;

  for (i=0; i<256; i++)
    s += string(16, (char)i);

  m->set_f_bytes(s);

  return m;
}

Message* test_enum()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  m->set_f_enum(test::MessagePrimitives::ENUM_1);
  return m;
}

Message* test_nested()
{
  test::MessageNested* m = new test::MessageNested;
  test::MessagePrimitives *m1;

  int i;

  for (i=0; i<10; i++) {
    m1 = m->add_f_nested2();
    m1->set_f_string("hello world\n");
  }

  return m;
}

Message* test_repeated_nested()
{
  test::MessageNested* m = new test::MessageNested;
  test::MessagePrimitives *m1;

  int i;

  for (i=0; i<10; i++) {
    m1 = m->add_f_nested2();
    m1->set_f_int32(i*i*i*i);
    m1->set_f_string("hello world\n");
  }

  return m;
}

Message* test_interleaved_nested()
{
  test::MessageNestedInterleaved* m = new test::MessageNestedInterleaved;
  test::MessagePrimitives *m1;

  m1 = m->add_f_nested2();
  m1->set_f_double(1e12);
  m1->set_f_float(1e9);
  m1->set_f_string("nested2\n");

  m->set_f_field3("test field3\n");

  m1 = m->add_f_nested4();
  m1->set_f_double(1e12);
  m1->set_f_float(1e9);
  m1->set_f_string("nested4\n");

  int i;

  for (i=0; i<10; i++)
    m->add_f_field5(i*i);

  return m;
}

Message* test_recursive_nested()
{
  test::MessageTreeNode* m = new test::MessageTreeNode;
  test::MessageTreeNode* m1 = m;

  int i;
  int depth = 10;

  for (i=0; i<depth; i++) {
    m1->set_f_name("child");
    m1->set_f_value( i & 1 ? "odd" : "even" );
    if (i<depth-1) m1 = m1->add_f_children();
  }

  return m;
}

Message* test_packed()
{
  test::MessagePacked* m = new test::MessagePacked;

  int i;

  for (i=0; i<10; i++)
  {
    m->add_f_double(11 * (double)i / 7);
    m->add_f_int64(i * i * i * i);
    m->add_f_int32(i * i * i * i);
    m->add_f_fixed64(i * i * i * i);
    m->add_f_fixed32(i * i * i * i);
  }

  return m;
}

/* --------------------------------------------------------- */


int main(int argc, char** argv)
{
  test_case_t tests[] = {
    { "double", test_double },
    { "zero int32", test_zero_int32 },
    { "positive int32", test_positive_int32 },
    { "negative int32", test_negative_int32 },
    { "zero sint32", test_zero_sint32 },
    { "positive sint32", test_positive_sint32 },
    { "negative sint32", test_negative_sint32 },
    { "fixed32", test_fixed32 },
    { "fixed64", test_fixed64 },
    { "max uint32", test_max_uint32 },
    { "max uint64", test_max_uint64 },
    { "boolean true", test_true_bool },
    { "boolean false", test_false_bool },
    { "string", test_string },
    { "empty string", test_empty_string },
    { "nulls string", test_nulls_string },
    { "utf8 string", test_utf8_string },
    { "long string", test_long_string },
    { "binary", test_bytes },
    { "enum", test_enum },
    { "nested", test_nested },
    { "repeated nested", test_repeated_nested },
    { "recursive nested", test_recursive_nested },
    { "packed", test_packed },
    { 0, 0 },
  };

  int i;

  for (i=0; tests[i].name; i++)
  {
    Message* msg = tests[i].fn();

    string pyout;
    stringstream binout;

    PythonFormat::PrintToString(*msg, &pyout);
    msg->SerializeToOstream(&binout);

    cout << tests[i].name << endl;
    cout << msg->GetDescriptor()->full_name() << endl;
    cout << pyout << endl;
    cout << "\"" << strings::CHexEscape(binout.str()) << "\"" << endl;
    cout << endl;

    delete msg;
  }

  return 0;
}

