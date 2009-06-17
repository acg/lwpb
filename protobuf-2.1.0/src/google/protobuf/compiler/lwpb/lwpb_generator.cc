// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: simon.kallweit@intefo.ch (Simon Kallweit)
//
// This module outputs lwpb protocol message dictionaries.

#include <utility>
#include <map>
#include <string>
#include <vector>

#include <google/protobuf/compiler/lwpb/lwpb_generator.h>
#include <google/protobuf/descriptor.pb.h>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/stubs/substitute.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace lwpb {

namespace {

// Gets the index of an element in a vector
template<typename InputIterator, typename EqualityComparable>
typename iterator_traits<InputIterator>::difference_type
Index(const InputIterator& begin, const InputIterator& end,
const EqualityComparable& item) {
return distance(begin, find(begin, end, item));
}


// Returns a copy of |filename| with any trailing ".protodevel" or ".proto
// suffix stripped.
// TODO(robinson): Unify with copy in compiler/cpp/internal/helpers.cc.
string StripProto(const string& filename) {
  const char* suffix = HasSuffixString(filename, ".protodevel")
      ? ".protodevel" : ".proto";
  return StripSuffixString(filename, suffix);
}


// Returns the Python module name expected for a given .proto filename.
string ModuleName(const string& filename) {
  string basename = StripProto(filename);
  StripString(&basename, "-", '_');
  StripString(&basename, "/", '.');
  return basename + "_pb2";
}

// Returns the Python module filename expected for a given .proto filename.
string ModuleFileName(const string& filename) {
  string basename = StripProto(filename);
  StripString(&basename, "-", '_');
  return basename + "_pb2";
}

// Returns the name of all containing types for descriptor,
// in order from outermost to innermost, followed by descriptor's
// own name.  Each name is separated by |separator|.
template <typename DescriptorT>
string NamePrefixedWithNestedTypes(const DescriptorT& descriptor,
                                   const string& separator) {
  string name = descriptor.name();
  for (const Descriptor* current = descriptor.containing_type();
       current != NULL; current = current->containing_type()) {
    name = current->name() + separator + name;
  }
  return name;
}


// Returns a Python literal giving the default value for a field.
// If the field specifies no explicit default value, we'll return
// the default default value for the field type (zero for numbers,
// empty string for strings, empty list for repeated fields, and
// None for non-repeated, composite fields).
//
// TODO(robinson): Unify with code from
// //compiler/cpp/internal/primitive_field.cc
// //compiler/cpp/internal/enum_field.cc
// //compiler/cpp/internal/string_field.cc
string StringifyDefaultValue(const FieldDescriptor& field) {
  if (field.is_repeated()) {
    return ".def.null = 0";
  }

  switch (field.cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
      return ".def.int32 = " + SimpleItoa(field.default_value_int32());
    case FieldDescriptor::CPPTYPE_UINT32:
      return ".def.uint32 = " + SimpleItoa(field.default_value_uint32());
    case FieldDescriptor::CPPTYPE_INT64:
      return ".def.int64 = " + SimpleItoa(field.default_value_int64());
    case FieldDescriptor::CPPTYPE_UINT64:
      return ".def.uint64 = " + SimpleItoa(field.default_value_uint64());
    case FieldDescriptor::CPPTYPE_DOUBLE:
      return ".def._double = " + SimpleDtoa(field.default_value_double());
    case FieldDescriptor::CPPTYPE_FLOAT:
      return ".def._float = " + SimpleFtoa(field.default_value_double());
    case FieldDescriptor::CPPTYPE_BOOL:
      return ".def.bool = " + field.default_value_bool() ? "True" : "False";
    case FieldDescriptor::CPPTYPE_ENUM:
      return ".def.int32 = " + SimpleItoa(field.default_value_enum()->number());
    case FieldDescriptor::CPPTYPE_STRING:
      return ".def.string = \"" + CEscape(field.default_value_string()) + "\"";
    case FieldDescriptor::CPPTYPE_MESSAGE:
      return ".def.null = 0";
  }
  // (We could add a default case above but then we wouldn't get the nice
  // compiler warning when a new type is added.)
  GOOGLE_LOG(FATAL) << "Not reached.";
  return "";
}

}  // namespace


Generator::Generator() : file_(NULL) {
}

Generator::~Generator() {
}

bool Generator::Generate(const FileDescriptor* file,
                         const string& parameter,
                         OutputDirectory* output_directory,
                         string* error) const {

  // Completely serialize all Generate() calls on this instance.  The
  // thread-safety constraints of the CodeGenerator interface aren't clear so
  // just be as conservative as possible.  It's easier to relax this later if
  // we need to, but I doubt it will be an issue.
  // TODO(kenton):  The proper thing to do would be to allocate any state on
  //   the stack and use that, so that the Generator class itself does not need
  //   to have any mutable members.  Then it is implicitly thread-safe.
  MutexLock lock(&mutex_);
  file_ = file;
  
  string filename = ModuleFileName(file->name());

  scoped_ptr<io::ZeroCopyOutputStream> h_output(output_directory->Open(filename + ".h"));
  GOOGLE_CHECK(h_output.get());
  io::Printer h_printer(h_output.get(), '$');
  h_printer_ = &h_printer;
  
  scoped_ptr<io::ZeroCopyOutputStream> c_output(output_directory->Open(filename + ".c"));
  GOOGLE_CHECK(c_output.get());
  io::Printer c_printer(c_output.get(), '$');
  c_printer_ = &c_printer;
  
  CreateEnumList();
  CreateMessageList();
  
  PrintHeader();
  
  PrintImports();
  
  PrintEnumDescriptors();
  PrintMessageDescriptors();
  
  PrintFooter();
  
  return (!h_printer.failed()) && (!c_printer.failed());
}


// Returns the name of the include guard.
string Generator::FileDescriptorIncludeGuard(
    const FileDescriptor& descriptor) const {
  string name = StripProto(descriptor.name());
  StripString(&name, "-", '_');
  StripString(&name, "/", '.');
  name = "__" + name + "_PB2_H__";
  UpperString(&name);
  return name;
}

// Returns the name of the message array.
string Generator::FileDescriptorMessageArray(
    const FileDescriptor& descriptor) const {
  string name = descriptor.package();    
  name = StringReplace(name, ".", "_", true);
  name = "lwpb_messages_" + name;
  LowerString(&name);
  return name;
}

// Returns the name of the message descriptor.
string Generator::MessageDescriptorId(
    const Descriptor& descriptor) const {
  string name = descriptor.full_name();
  name = StringReplace(name, ".", "_", true);
  //UpperString(&name);
  return name;
}

// Returns the name of the message descriptor pointer.
string Generator::MessageDescriptorPtr(
    const Descriptor& descriptor) const {
  string array = FileDescriptorMessageArray(*descriptor.file());
  string ptr = "(&" + array + "[" + SimpleItoa(GetMessageIndex(&descriptor)) + "])";
  return ptr;
}

// Returns the name of the message's field array.
string Generator::MessageDescriptorFieldArray(
    const Descriptor& descriptor) const {
  string name = descriptor.full_name();
  name = StringReplace(name, ".", "_", true);
  name = "lwpb_fields_" + name;
  LowerString(&name);
  return name;
}

// Returns the name of the field descriptor.
string Generator::FieldDescriptorId(
    const FieldDescriptor& descriptor) const {
  string name = descriptor.full_name();
  name = StringReplace(name, ".", "_", true);
  return name;
}

// Returns the name of the field descriptor pointer.
string Generator::FieldDescriptorPtr(
    const FieldDescriptor& descriptor) const {
  string array = MessageDescriptorFieldArray(*descriptor.containing_type());
  string ptr = "(&" + array + "[" + SimpleItoa(descriptor.index()) + "])";
  return ptr;
}

string Generator::FieldDescriptorOptLabel(const FieldDescriptor& field) const {
  switch (field.label()) {
  case FieldDescriptor::LABEL_OPTIONAL: return "LWPB_OPTIONAL";
  case FieldDescriptor::LABEL_REQUIRED: return "LWPB_REQUIRED";
  case FieldDescriptor::LABEL_REPEATED: return "LWPB_REPEATED";
  }
    
  GOOGLE_LOG(FATAL) << "Not reached.";
  return "";
}

string Generator::FieldDescriptorOptTyp(const FieldDescriptor& field) const {
  switch (field.type()) {
  case FieldDescriptor::TYPE_DOUBLE:    return "LWPB_DOUBLE";
  case FieldDescriptor::TYPE_FLOAT:     return "LWPB_FLOAT";
  case FieldDescriptor::TYPE_INT32:     return "LWPB_INT32";
  case FieldDescriptor::TYPE_INT64:     return "LWPB_INT64";
  case FieldDescriptor::TYPE_UINT32:    return "LWPB_UINT64";
  case FieldDescriptor::TYPE_UINT64:    return "LWPB_UINT64";
  case FieldDescriptor::TYPE_SINT32:    return "LWPB_SINT32";
  case FieldDescriptor::TYPE_SINT64:    return "LWPB_SINT64";
  case FieldDescriptor::TYPE_FIXED32:   return "LWPB_FIXED32";
  case FieldDescriptor::TYPE_FIXED64:   return "LWPB_FIXED64";
  case FieldDescriptor::TYPE_SFIXED32:  return "LWPB_SFIXED32";
  case FieldDescriptor::TYPE_SFIXED64:  return "LWPB_SFIXED64";
  case FieldDescriptor::TYPE_BOOL:      return "LWPB_BOOL";
  case FieldDescriptor::TYPE_STRING:    return "LWPB_STRING";
  case FieldDescriptor::TYPE_BYTES:     return "LWPB_BYTES";
  // TODO: we don't support groups
  case FieldDescriptor::TYPE_GROUP:     return "LWPB_GROUP";
  case FieldDescriptor::TYPE_MESSAGE:   return "LWPB_MESSAGE";
  case FieldDescriptor::TYPE_ENUM:      return "LWPB_ENUM";
  }
  
  GOOGLE_LOG(FATAL) << "Not reached.";
  return "";
}

string Generator::FieldDescriptorOptFlags(const FieldDescriptor& field) const {
  string flags = "0";
  if (field.has_default_value())
    flags += " | LWPB_HAS_DEFAULT";
  if (field.options().packed())
    flags += " | LWPB_IS_PACKED";
  if (field.options().deprecated())
    flags += " | LWPB_IS_DEPRECATED";
  
  return flags;
}

// Creates a flat vector |enums| of all enum descriptors.
void Generator::CreateEnumList() const {
  for (int i = 0; i < file_->enum_type_count(); ++i)
    enums_.push_back(file_->enum_type(i));
  for (int i = 0; i < file_->message_type_count(); ++i)
    AddNestedEnums(*file_->message_type(i));
}

// Adds all nested enum descriptors to the |enums| vector.
void Generator::AddNestedEnums(
    const Descriptor& containing_descriptor) const {
  for (int i = 0; i < containing_descriptor.enum_type_count(); ++i)
    enums_.push_back(containing_descriptor.enum_type(i));
  for (int i = 0; i < containing_descriptor.nested_type_count(); ++i)
    AddNestedEnums(*containing_descriptor.nested_type(i));
}

// Creates a flat vector |messages| of all message descriptors.
void Generator::CreateMessageList() const {
  for (int i = 0; i < file_->message_type_count(); ++i)
    messages_.push_back(file_->message_type(i));
  for (int i = 0; i < file_->message_type_count(); ++i)
    AddNestedMessages(*file_->message_type(i));
}

// Adds all nested message descriptors to the |messages| vector.
void Generator::AddNestedMessages(
    const Descriptor& containing_descriptor) const {
  for (int i = 0; i < containing_descriptor.nested_type_count(); ++i)
    messages_.push_back(containing_descriptor.nested_type(i));
  for (int i = 0; i < containing_descriptor.nested_type_count(); ++i)
    AddNestedMessages(*containing_descriptor.nested_type(i));
}

// Returns the index of a message descriptor in the flat vector |messages_|.
int Generator::GetMessageIndex(const Descriptor* message_descriptor) const {
  return Index(messages_.begin(), messages_.end(), message_descriptor);
}


void Generator::PrintHeader() const {
  // Print .h header
  {
    map<string, string> m;
    m["guard"] = FileDescriptorIncludeGuard(*file_);
    h_printer_->Print(m,
        "// Generated by the protocol buffer compiler.  DO NOT EDIT!\n"
        "\n"
        "#ifndef $guard$\n"
        "#define $guard$\n"
        "\n"
        "#include <lwpb/lwpb.h>\n"
        "\n");
  }
  // Print .c header
  {
    map<string, string> m;
    m["include"] = ModuleFileName(file_->name()) + ".h";
    c_printer_->Print(m,
        "// Generated by the protocol buffer compiler.  DO NOT EDIT!\n"
        "\n"
        "#include <lwpb/lwpb.h>\n"
        "\n"
        "#include \"$include$\"\n"
        "\n");
  }
}

void Generator::PrintFooter() const {
  // Print .h footer
  {
    map<string, string> m;
    m["guard"] = FileDescriptorIncludeGuard(*file_);
    h_printer_->Print(m,
        "#endif // $guard$\n");
  }
  // Print .c footer
  {
  }
}

// Prints Python imports for all modules imported by |file|.
void Generator::PrintImports() const {
  for (int i = 0; i < file_->dependency_count(); ++i) {
    string module_name = ModuleFileName(file_->dependency(i)->name()) + ".h";
    h_printer_->Print("#include \"$module$\"\n", "module", module_name);
  }
  h_printer_->Print("\n");
}

void Generator::PrintEnumDescriptors() const {
  for (int i = 0; i < enums_.size(); ++i)
    PrintEnumDescriptor(*enums_[i]);
}

void Generator::PrintEnumDescriptor(
    const EnumDescriptor& enum_descriptor) const {
  string enum_name = NamePrefixedWithNestedTypes(enum_descriptor, ".");
  h_printer_->Print("// '$name$' enumeration values\n", "name", enum_name);

  string descriptor_name = ModuleLevelDescriptorName(enum_descriptor);
  for (int i = 0; i < enum_descriptor.value_count(); ++i) {
    const EnumValueDescriptor &value_descriptor = *enum_descriptor.value(i);
    string name = value_descriptor.full_name();
    name = StringReplace(name, ".", "_", true);
    UpperString(&name);
    string number = SimpleItoa(value_descriptor.number());
    h_printer_->Print("#define $name$ $number$\n",
                      "name", name, "number", number);
  }
  h_printer_->Print("\n");
}

void Generator::PrintMessageDescriptors() const {
  // Print reference to message array
  string array_name = FileDescriptorMessageArray(*file_);
  h_printer_->Print("extern const struct lwpb_msg_desc $array$[];\n",
                    "array", array_name);
  h_printer_->Print("\n");
    
  // Print message id's
  h_printer_->Print("// Message descriptor pointers\n");
  for (int i = 0; i < messages_.size(); ++i) {
    string id = MessageDescriptorId(*messages_[i]);
    string ptr = MessageDescriptorPtr(*messages_[i]);
    h_printer_->Print("#define $id$ $ptr$\n", "id", id, "ptr", ptr);
  }
  h_printer_->Print("\n");
    
  // Print field descriptions
  for (int i = 0; i < messages_.size(); ++i)
    PrintDescriptorFields(*messages_[i]);
  
  // Print message descriptions
  c_printer_->Print("// Message descriptors\n");
  c_printer_->Print("const struct lwpb_msg_desc $name$[] = {\n", "name", array_name);
  for (int i = 0; i < messages_.size(); ++i) {
    map<string, string> m;
    m["name"] = NamePrefixedWithNestedTypes(*messages_[i], ".");
    m["num_fields"] = SimpleItoa(messages_[i]->field_count());
    m["fields"] = MessageDescriptorFieldArray(*messages_[i]);
    LowerString(&m["fields"]);
    c_printer_->Print(m,
        "    {\n"
        "        .num_fields = $num_fields$,\n"
        "        .fields = $fields$,\n"
        "#ifdef LWPB_MESSAGE_NAMES\n"
        "        .name = \"$name$\",\n"
        "#endif\n"
        "    },\n");
  }
  c_printer_->Print("};\n");
  c_printer_->Print("\n");
}

void Generator::PrintDescriptorFields(
    const Descriptor& message_descriptor) const {
  // Print reference to field array
  string array_name = MessageDescriptorFieldArray(message_descriptor);
  h_printer_->Print("extern const struct lwpb_field_desc $array$[];\n",
                    "array", array_name);
  h_printer_->Print("\n");
    
  // Print field id's
  string message_name = NamePrefixedWithNestedTypes(message_descriptor, ".");
  h_printer_->Print("// '$name$' field descriptor pointers\n", "name", message_name);
  
  string descriptor_name = ModuleLevelDescriptorName(message_descriptor);
  for (int i = 0; i < message_descriptor.field_count(); ++i) {
    const FieldDescriptor &field_descriptor = *message_descriptor.field(i);
    string id = FieldDescriptorId(field_descriptor);
    string ptr = FieldDescriptorPtr(field_descriptor);
    h_printer_->Print("#define $id$ $ptr$\n", "id", id, "ptr", ptr);
  }
  h_printer_->Print("\n");
  
  // Print field descriptor array
  c_printer_->Print("// '$name$' field descriptors\n", "name", message_name);
  c_printer_->Print("const struct lwpb_field_desc $name$[] = {\n", "name", array_name);
  for (int i = 0; i < message_descriptor.field_count(); ++i) {
    const FieldDescriptor &field_descriptor = *message_descriptor.field(i);
    string name = FieldDescriptorId(field_descriptor);
    
    map <string, string> m;
    m["number"] = SimpleItoa(field_descriptor.number());
    m["label"] = FieldDescriptorOptLabel(field_descriptor);
    m["typ"] = FieldDescriptorOptTyp(field_descriptor);
    m["flags"] = FieldDescriptorOptFlags(field_descriptor);
    m["msg_desc"] = field_descriptor.type() == FieldDescriptor::TYPE_MESSAGE ?
                    MessageDescriptorId(*field_descriptor.message_type()) : "0";
    m["name"] = field_descriptor.name();
    m["default"] = StringifyDefaultValue(field_descriptor);
    c_printer_->Print(m,
        "    {\n"
        "        .number = $number$,\n"
        "        .opts.label = $label$,\n"
        "        .opts.typ = $typ$,\n"
        "        .opts.flags = $flags$,\n"
        "        .msg_desc = $msg_desc$,\n"
        "#ifdef LWPB_FIELD_NAMES\n"
        "        .name = \"$name$\",\n"
        "#endif\n"
        "#ifdef LWPB_FIELD_DEFAULTS\n"
        "        $default$,\n"
        "#endif\n"
        "    },\n");
  }
  c_printer_->Print("};\n");
  c_printer_->Print("\n");
}

string Generator::OptionsValue(
    const string& class_name, const string& serialized_options) const {
  if (serialized_options.length() == 0 || GeneratingDescriptorProto()) {
    return "None";
  } else {
    string full_class_name = "descriptor_pb2." + class_name;
    return "descriptor._ParseOptions(" + full_class_name + "(), '"
        + CEscape(serialized_options)+ "')";
  }
}

bool Generator::GeneratingDescriptorProto() const {
  return file_->name() == "google/protobuf/descriptor.proto";
}

// Returns the unique Python module-level identifier given to a descriptor.
// This name is module-qualified iff the given descriptor describes an
// entity that doesn't come from the current file.
template <typename DescriptorT>
string Generator::ModuleLevelDescriptorName(
    const DescriptorT& descriptor) const {
  // FIXME(robinson):
  // We currently don't worry about collisions with underscores in the type
  // names, so these would collide in nasty ways if found in the same file:
  //   OuterProto.ProtoA.ProtoB
  //   OuterProto_ProtoA.ProtoB  # Underscore instead of period.
  // As would these:
  //   OuterProto.ProtoA_.ProtoB
  //   OuterProto.ProtoA._ProtoB  # Leading vs. trailing underscore.
  // (Contrived, but certainly possible).
  //
  // The C++ implementation doesn't guard against this either.  Leaving
  // it for now...
  string name = NamePrefixedWithNestedTypes(descriptor, "_");
  // We now have the name relative to its own module.  Also qualify with
  // the module name iff this descriptor is from a different .proto file.
  if (descriptor.file() != file_) {
    name = ModuleName(descriptor.file()->name()) + "_" + name;
  }
  UpperString(&name);
  return name;
}

// Returns the name of the message class itself, not the descriptor.
// Like ModuleLevelDescriptorName(), module-qualifies the name iff
// the given descriptor describes an entity that doesn't come from
// the current file.
string Generator::ModuleLevelMessageName(const Descriptor& descriptor) const {
  string name = NamePrefixedWithNestedTypes(descriptor, ".");
  if (descriptor.file() != file_) {
    name = ModuleName(descriptor.file()->name()) + "." + name;
  }
  return name;
}

// Returns the unique Python module-level identifier given to a service
// descriptor.
string Generator::ModuleLevelServiceDescriptorName(
    const ServiceDescriptor& descriptor) const {
  string name = descriptor.name();
  UpperString(&name);
  name = "_" + name;
  if (descriptor.file() != file_) {
    name = ModuleName(descriptor.file()->name()) + "." + name;
  }
  return name;
}

}  // namespace lwpb
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
