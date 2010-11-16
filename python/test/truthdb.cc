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
  m->set_f_double(1.2345);
  return m;
}

Message* test_string()
{
  test::MessagePrimitives* m = new test::MessagePrimitives;
  m->set_f_string("hello world");
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
  m1->set_f_string("hello world\n");

  m->set_f_field3("test field3\n");

  int i;

  for (i=0; i<10; i++)
    m->add_f_field5(i*i);

  return m;
}

Message* test_packed()
{
  test::MessagePacked* m = new test::MessagePacked;

  int i;

  for (i=0; i<10; i++)
  {
    m->add_f_double(11 * (double)i / 7);
    m->add_f_float(11 * (double)i / 7);
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
    { "string", test_string },
    { "long string", test_long_string },
    { "binary", test_bytes },
    { "enum", test_enum },
    { "nested", test_nested },
    { "repeated nested", test_repeated_nested },
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

