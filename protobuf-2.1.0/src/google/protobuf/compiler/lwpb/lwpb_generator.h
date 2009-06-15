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
// Generates Python code for a given .proto file.

#ifndef GOOGLE_PROTOBUF_COMPILER_LWPB_GENERATOR_H__
#define GOOGLE_PROTOBUF_COMPILER_LWPB_GENERATOR_H__

#include <string>
#include <vector>

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/stubs/common.h>

namespace google {
namespace protobuf {

class Descriptor;
class EnumDescriptor;
class EnumValueDescriptor;
class FieldDescriptor;
class ServiceDescriptor;

namespace io { class Printer; }

namespace compiler {
namespace lwpb {

// CodeGenerator implementation for generated lwpb protocol buffer classes.
// If you create your own protocol compiler binary and you want it to support
// Python output, you can do so by registering an instance of this
// CodeGenerator with the CommandLineInterface in your main() function.
class LIBPROTOC_EXPORT Generator : public CodeGenerator {
 public:
  Generator();
  virtual ~Generator();

  // CodeGenerator methods.
  virtual bool Generate(const FileDescriptor* file,
                        const string& parameter,
                        OutputDirectory* output_directory,
                        string* error) const;

 private:
  void CreateEnumList() const;
  void AddNestedEnums(const Descriptor& containing_descriptor) const;

  void CreateMessageList() const;
  void AddNestedMessages(const Descriptor& containing_descriptor) const;
  int GetMessageIndex(const Descriptor* message_descriptor) const;
  
  void PrintImports() const;
  void PrintEnumDescriptors() const;
  void PrintEnumDescriptor(const EnumDescriptor& enum_descriptor) const;
  void PrintMessageDescriptors() const;
  void PrintDescriptorFields(const Descriptor& message_descriptor) const;
  string StringifyFieldType(const FieldDescriptor& field) const;

  string OptionsValue(const string& class_name,
                      const string& serialized_options) const;
  bool GeneratingDescriptorProto() const;

  template <typename DescriptorT>
  string ModuleLevelDescriptorName(const DescriptorT& descriptor) const;
  string ModuleLevelMessageName(const Descriptor& descriptor) const;
  string ModuleLevelServiceDescriptorName(
      const ServiceDescriptor& descriptor) const;

  // Very coarse-grained lock to ensure that Generate() is reentrant.
  // Guards file_ and printer_.
  mutable Mutex mutex_;
  mutable const FileDescriptor* file_;  // Set in Generate().  Under mutex_.
  mutable io::Printer* printer_;  // Set in Generate().  Under mutex_.

  mutable vector<const EnumDescriptor*> enums_;
  mutable vector<const Descriptor*> messages_;

  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Generator);
};

}  // namespace lwpb
}  // namespace compiler
}  // namespace protobuf

}  // namespace google
#endif  // GOOGLE_PROTOBUF_COMPILER_LWPB_GENERATOR_H__
