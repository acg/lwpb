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

// Author: alangrow@gmail.com (Alan Grow)
//  Based on text_format.cc by jschorr@google.com (Joseph Schorr)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include "python_format.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stack>
#include <limits>


#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/unknown_field_set.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/io/tokenizer.h>
#include <google/protobuf/stubs/strutil.h>

namespace google {
namespace protobuf {

string Message::DebugString() const {
  string debug_string;

  PythonFormat::PrintToString(*this, &debug_string);

  return debug_string;
}

string Message::ShortDebugString() const {
  string debug_string;

  PythonFormat::Printer printer;
  printer.SetSingleLineMode(true);

  printer.PrintToString(*this, &debug_string);
  // Single line mode currently might have an extra space at the end.
  if (debug_string.size() > 0 &&
      debug_string[debug_string.size() - 1] == ' ') {
    debug_string.resize(debug_string.size() - 1);
  }

  return debug_string;
}

string Message::Utf8DebugString() const {
  string debug_string;

  PythonFormat::Printer printer;
  printer.SetUseUtf8StringEscaping(true);

  printer.PrintToString(*this, &debug_string);

  return debug_string;
}

void Message::PrintDebugString() const {
  printf("%s", DebugString().c_str());
}

// ===========================================================================
// Internal class for writing text to the io::ZeroCopyOutputStream. Adapted
// from the Printer found in //google/protobuf/io/printer.h
class PythonFormat::Printer::PythonGenerator {
 public:
  explicit PythonGenerator(io::ZeroCopyOutputStream* output,
                         int initial_indent_level)
    : output_(output),
      buffer_(NULL),
      buffer_size_(0),
      at_start_of_line_(true),
      failed_(false),
      indent_(""),
      initial_indent_level_(initial_indent_level) {
    indent_.resize(initial_indent_level_ * 2, ' ');
  }

  ~PythonGenerator() {
    // Only BackUp() if we're sure we've successfully called Next() at least
    // once.
    if (buffer_size_ > 0) {
      output_->BackUp(buffer_size_);
    }
  }

  // Indent text by two spaces.  After calling Indent(), two spaces will be
  // inserted at the beginning of each line of text.  Indent() may be called
  // multiple times to produce deeper indents.
  void Indent() {
    indent_ += "  ";
  }

  // Reduces the current indent level by two spaces, or crashes if the indent
  // level is zero.
  void Outdent() {
    if (indent_.empty() ||
        indent_.size() < initial_indent_level_ * 2) {
      GOOGLE_LOG(DFATAL) << " Outdent() without matching Indent().";
      return;
    }

    indent_.resize(indent_.size() - 2);
  }

  // Print text to the output stream.
  void Print(const string& str) {
    Print(str.data(), str.size());
  }

  // Print text to the output stream.
  void Print(const char* text) {
    Print(text, strlen(text));
  }

  // Print text to the output stream.
  void Print(const char* text, int size) {
    int pos = 0;  // The number of bytes we've written so far.

    for (int i = 0; i < size; i++) {
      if (text[i] == '\n') {
        // Saw newline.  If there is more text, we may need to insert an indent
        // here.  So, write what we have so far, including the '\n'.
        Write(text + pos, i - pos + 1);
        pos = i + 1;

        // Setting this true will cause the next Write() to insert an indent
        // first.
        at_start_of_line_ = true;
      }
    }

    // Write the rest.
    Write(text + pos, size - pos);
  }

  // True if any write to the underlying stream failed.  (We don't just
  // crash in this case because this is an I/O failure, not a programming
  // error.)
  bool failed() const { return failed_; }

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(PythonGenerator);

  void Write(const char* data, int size) {
    if (failed_) return;
    if (size == 0) return;

    if (at_start_of_line_) {
      // Insert an indent.
      at_start_of_line_ = false;
      Write(indent_.data(), indent_.size());
      if (failed_) return;
    }

    while (size > buffer_size_) {
      // Data exceeds space in the buffer.  Copy what we can and request a
      // new buffer.
      memcpy(buffer_, data, buffer_size_);
      data += buffer_size_;
      size -= buffer_size_;
      void* void_buffer;
      failed_ = !output_->Next(&void_buffer, &buffer_size_);
      if (failed_) return;
      buffer_ = reinterpret_cast<char*>(void_buffer);
    }

    // Buffer is big enough to receive the data; copy it.
    memcpy(buffer_, data, size);
    buffer_ += size;
    buffer_size_ -= size;
  }

  io::ZeroCopyOutputStream* const output_;
  char* buffer_;
  int buffer_size_;
  bool at_start_of_line_;
  bool failed_;

  string indent_;
  int initial_indent_level_;
};

// ===========================================================================

PythonFormat::Printer::Printer()
  : initial_indent_level_(0),
    single_line_mode_(true),
    use_short_repeated_primitives_(false),
    utf8_string_escaping_(true) {}

PythonFormat::Printer::~Printer() {}

bool PythonFormat::Printer::PrintToString(const Message& message,
                                        string* output) {
  GOOGLE_DCHECK(output) << "output specified is NULL";

  output->clear();
  io::StringOutputStream output_stream(output);

  bool result = Print(message, &output_stream);

  return result;
}

bool PythonFormat::Printer::PrintUnknownFieldsToString(
    const UnknownFieldSet& unknown_fields,
    string* output) {
  GOOGLE_DCHECK(output) << "output specified is NULL";

  output->clear();
  io::StringOutputStream output_stream(output);
  return PrintUnknownFields(unknown_fields, &output_stream);
}

bool PythonFormat::Printer::Print(const Message& message,
                                io::ZeroCopyOutputStream* output) {
  PythonGenerator generator(output, initial_indent_level_);

  Print(message, generator);

  // Output false if the generator failed internally.
  return !generator.failed();
}

bool PythonFormat::Printer::PrintUnknownFields(
    const UnknownFieldSet& unknown_fields,
    io::ZeroCopyOutputStream* output) {
  PythonGenerator generator(output, initial_indent_level_);

  PrintUnknownFields(unknown_fields, generator);

  // Output false if the generator failed internally.
  return !generator.failed();
}

void PythonFormat::Printer::Print(const Message& message,
                                PythonGenerator& generator) {
  const Reflection* reflection = message.GetReflection();
  vector<const FieldDescriptor*> fields;
  reflection->ListFields(message, &fields);
  int count = fields.size();

  generator.Print("{");

  for (int i = 0; i < count; i++) {
    PrintField(message, reflection, fields[i], generator);
    if (i < count-1) generator.Print(",");
  }

  PrintUnknownFields(reflection->GetUnknownFields(message), generator);

  generator.Print("}");
}

void PythonFormat::Printer::PrintFieldValueToString(
    const Message& message,
    const FieldDescriptor* field,
    int index,
    string* output) {

  GOOGLE_DCHECK(output) << "output specified is NULL";

  output->clear();
  io::StringOutputStream output_stream(output);
  PythonGenerator generator(&output_stream, initial_indent_level_);

  PrintFieldValue(message, message.GetReflection(), field, index, generator);
}

void PythonFormat::Printer::PrintField(const Message& message,
                                     const Reflection* reflection,
                                     const FieldDescriptor* field,
                                     PythonGenerator& generator) {
  int count = 0;

  if (field->is_repeated()) {
    count = reflection->FieldSize(message, field);
  } else if (reflection->HasField(message, field)) {
    count = 1;
  }

  PrintFieldName(message, reflection, field, generator);
  generator.Print(":");

  if (field->is_repeated()) {
    generator.Print("[");
  }

  for (int j = 0; j < count; ++j) {

    // Write the field value.
    int field_index = j;
    if (!field->is_repeated()) {
      field_index = -1;
    }

    PrintFieldValue(message, reflection, field, field_index, generator);

    if (j<count-1)
      generator.Print(",");
  }

  if (field->is_repeated()) {
    generator.Print("]");
  }
}

void PythonFormat::Printer::PrintFieldName(const Message& message,
                                         const Reflection* reflection,
                                         const FieldDescriptor* field,
                                         PythonGenerator& generator) {
  if (field->is_extension()) {
    generator.Print("[");
    // We special-case MessageSet elements for compatibility with proto1.
    if (field->containing_type()->options().message_set_wire_format()
        && field->type() == FieldDescriptor::TYPE_MESSAGE
        && field->is_optional()
        && field->extension_scope() == field->message_type()) {
      generator.Print(field->message_type()->full_name());
    } else {
      generator.Print(field->full_name());
    }
    generator.Print("]");
  } else {
    generator.Print("\"");
    if (field->type() == FieldDescriptor::TYPE_GROUP) {
      // Groups must be serialized with their original capitalization.
      generator.Print(field->message_type()->name());
    } else {
      generator.Print(field->name());
    }
    generator.Print("\"");
  }
}

void PythonFormat::Printer::PrintFieldValue(
    const Message& message,
    const Reflection* reflection,
    const FieldDescriptor* field,
    int index,
    PythonGenerator& generator) {
  GOOGLE_DCHECK(field->is_repeated() || (index == -1))
      << "Index must be -1 for non-repeated fields";

  switch (field->cpp_type()) {
#define OUTPUT_FIELD(CPPTYPE, METHOD, TO_STRING)                             \
      case FieldDescriptor::CPPTYPE_##CPPTYPE:                               \
        generator.Print(TO_STRING(field->is_repeated() ?                     \
          reflection->GetRepeated##METHOD(message, field, index) :           \
          reflection->Get##METHOD(message, field)));                         \
        break;                                                               \

      OUTPUT_FIELD( INT32,  Int32, SimpleItoa);
      OUTPUT_FIELD( INT64,  Int64, SimpleItoa);
      OUTPUT_FIELD(UINT32, UInt32, SimpleItoa);
      OUTPUT_FIELD(UINT64, UInt64, SimpleItoa);
      OUTPUT_FIELD( FLOAT,  Float, SimpleFtoa);
      OUTPUT_FIELD(DOUBLE, Double, SimpleDtoa);
#undef OUTPUT_FIELD

      case FieldDescriptor::CPPTYPE_STRING: {
        string scratch;
        const string& value = field->is_repeated() ?
            reflection->GetRepeatedStringReference(
              message, field, index, &scratch) :
            reflection->GetStringReference(message, field, &scratch);

        generator.Print("\"");
        if (utf8_string_escaping_ &&
            !field->type() == FieldDescriptor::TYPE_BYTES) {
          generator.Print(strings::Utf8SafeCEscape(value));
        } else {
          generator.Print(CEscape(value));
        }
        generator.Print("\"");

        break;
      }

      case FieldDescriptor::CPPTYPE_BOOL:
        if (field->is_repeated()) {
          generator.Print(reflection->GetRepeatedBool(message, field, index)
                          ? "true" : "false");
        } else {
          generator.Print(reflection->GetBool(message, field)
                          ? "true" : "false");
        }
        break;

      case FieldDescriptor::CPPTYPE_ENUM:
        generator.Print(SimpleItoa(field->is_repeated() ?
          reflection->GetRepeatedEnum(message, field, index)->number() :
          reflection->GetEnum(message, field)->number()));
        break;

      case FieldDescriptor::CPPTYPE_MESSAGE:
        Print(field->is_repeated() ?
                reflection->GetRepeatedMessage(message, field, index) :
                reflection->GetMessage(message, field),
              generator);
        break;
  }
}

/* static */ bool PythonFormat::Print(const Message& message,
                                    io::ZeroCopyOutputStream* output) {
  return Printer().Print(message, output);
}

/* static */ bool PythonFormat::PrintUnknownFields(
    const UnknownFieldSet& unknown_fields,
    io::ZeroCopyOutputStream* output) {
  return Printer().PrintUnknownFields(unknown_fields, output);
}

/* static */ bool PythonFormat::PrintToString(
    const Message& message, string* output) {
  return Printer().PrintToString(message, output);
}

/* static */ bool PythonFormat::PrintUnknownFieldsToString(
    const UnknownFieldSet& unknown_fields, string* output) {
  return Printer().PrintUnknownFieldsToString(unknown_fields, output);
}

/* static */ void PythonFormat::PrintFieldValueToString(
    const Message& message,
    const FieldDescriptor* field,
    int index,
    string* output) {
  return Printer().PrintFieldValueToString(message, field, index, output);
}

// Prints an integer as hex with a fixed number of digits dependent on the
// integer type.
template<typename IntType>
static string PaddedHex(IntType value) {
  string result;
  result.reserve(sizeof(value) * 2);
  for (int i = sizeof(value) * 2 - 1; i >= 0; i--) {
    result.push_back(int_to_hex_digit(value >> (i*4) & 0x0F));
  }
  return result;
}

void PythonFormat::Printer::PrintUnknownFields(
    const UnknownFieldSet& unknown_fields, PythonGenerator& generator) {
  for (int i = 0; i < unknown_fields.field_count(); i++) {
    const UnknownField& field = unknown_fields.field(i);
    string field_number = SimpleItoa(field.number());

    switch (field.type()) {
      case UnknownField::TYPE_VARINT:
        generator.Print(field_number);
        generator.Print(": ");
        generator.Print(SimpleItoa(field.varint()));
        generator.Print(" ");
        break;
      case UnknownField::TYPE_FIXED32: {
        generator.Print(field_number);
        generator.Print(": 0x");
        char buffer[kFastToBufferSize];
        generator.Print(FastHex32ToBuffer(field.fixed32(), buffer));
        generator.Print(" ");
        break;
      }
      case UnknownField::TYPE_FIXED64: {
        generator.Print(field_number);
        generator.Print(": 0x");
        char buffer[kFastToBufferSize];
        generator.Print(FastHex64ToBuffer(field.fixed64(), buffer));
        generator.Print(" ");
        break;
      }
      case UnknownField::TYPE_LENGTH_DELIMITED: {
        generator.Print(field_number);
        const string& value = field.length_delimited();
        UnknownFieldSet embedded_unknown_fields;
        if (!value.empty() && embedded_unknown_fields.ParseFromString(value)) {
          // This field is parseable as a Message.
          // So it is probably an embedded message.
          generator.Print(" { ");
          PrintUnknownFields(embedded_unknown_fields, generator);
          generator.Print("} ");
        } else {
          // This field is not parseable as a Message.
          // So it is probably just a plain string.
          generator.Print(": \"");
          generator.Print(CEscape(value));
          generator.Print("\"");
          generator.Print(" ");
        }
        break;
      }
      case UnknownField::TYPE_GROUP:
        generator.Print(field_number);
        generator.Print(" { ");
        PrintUnknownFields(field.group(), generator);
        generator.Print("} ");
        break;
    }
  }
}

}  // namespace protobuf
}  // namespace google
