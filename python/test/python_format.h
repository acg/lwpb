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

#ifndef GOOGLE_PROTOBUF_PYTHON_FORMAT_H__
#define GOOGLE_PROTOBUF_PYTHON_FORMAT_H__

#include <string>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

namespace google {
namespace protobuf {

namespace io {
  class ErrorCollector;      // tokenizer.h
}

// This class implements protocol buffer text format.  Printing and parsing
// protocol messages in text format is useful for debugging and human editing
// of messages.
//
// This class is really a namespace that contains only static methods.
class LIBPROTOBUF_EXPORT PythonFormat {
 public:
  // Outputs a textual representation of the given message to the given
  // output stream.
  static bool Print(const Message& message, io::ZeroCopyOutputStream* output);

  // Print the fields in an UnknownFieldSet.  They are printed by tag number
  // only.  Embedded messages are heuristically identified by attempting to
  // parse them.
  static bool PrintUnknownFields(const UnknownFieldSet& unknown_fields,
                                 io::ZeroCopyOutputStream* output);

  // Like Print(), but outputs directly to a string.
  static bool PrintToString(const Message& message, string* output);

  // Like PrintUnknownFields(), but outputs directly to a string.
  static bool PrintUnknownFieldsToString(const UnknownFieldSet& unknown_fields,
                                         string* output);

  // Outputs a textual representation of the value of the field supplied on
  // the message supplied. For non-repeated fields, an index of -1 must
  // be supplied. Note that this method will print the default value for a
  // field if it is not set.
  static void PrintFieldValueToString(const Message& message,
                                      const FieldDescriptor* field,
                                      int index,
                                      string* output);

  // Class for those users which require more fine-grained control over how
  // a protobuffer message is printed out.
  class LIBPROTOBUF_EXPORT Printer {
   public:
    Printer();
    ~Printer();

    // Like PythonFormat::Print
    bool Print(const Message& message, io::ZeroCopyOutputStream* output);
    // Like PythonFormat::PrintUnknownFields
    bool PrintUnknownFields(const UnknownFieldSet& unknown_fields,
                            io::ZeroCopyOutputStream* output);
    // Like PythonFormat::PrintToString
    bool PrintToString(const Message& message, string* output);
    // Like PythonFormat::PrintUnknownFieldsToString
    bool PrintUnknownFieldsToString(const UnknownFieldSet& unknown_fields,
                                    string* output);
    // Like PythonFormat::PrintFieldValueToString
    void PrintFieldValueToString(const Message& message,
                                 const FieldDescriptor* field,
                                 int index,
                                 string* output);

    // Adjust the initial indent level of all output.  Each indent level is
    // equal to two spaces.
    void SetInitialIndentLevel(int indent_level) {
      initial_indent_level_ = indent_level;
    }

    // If printing in single line mode, then the entire message will be output
    // on a single line with no line breaks.
    void SetSingleLineMode(bool single_line_mode) {
      single_line_mode_ = single_line_mode;
    }

    // Set true to print repeated primitives in a format like:
    //   field_name: [1, 2, 3, 4]
    // instead of printing each value on its own line.  Short format applies
    // only to primitive values -- i.e. everything except strings and
    // sub-messages/groups.  Note that at present this format is not recognized
    // by the parser.
    void SetUseShortRepeatedPrimitives(bool use_short_repeated_primitives) {
      use_short_repeated_primitives_ = use_short_repeated_primitives;
    }

    // Set true to output UTF-8 instead of ASCII.  The only difference
    // is that bytes >= 0x80 in string fields will not be escaped,
    // because they are assumed to be part of UTF-8 multi-byte
    // sequences.
    void SetUseUtf8StringEscaping(bool as_utf8) {
      utf8_string_escaping_ = as_utf8;
    }

   private:
    // Forward declaration of an internal class used to print the text
    // output to the OutputStream (see text_format.cc for implementation).
    class PythonGenerator;

    // Internal Print method, used for writing to the OutputStream via
    // the PythonGenerator class.
    void Print(const Message& message,
               PythonGenerator& generator);

    // Print a single field.
    void PrintField(const Message& message,
                    const Reflection* reflection,
                    const FieldDescriptor* field,
                    PythonGenerator& generator);

    // Print a repeated primitive field in short form.
    void PrintShortRepeatedField(const Message& message,
                                 const Reflection* reflection,
                                 const FieldDescriptor* field,
                                 PythonGenerator& generator);

    // Print the name of a field -- i.e. everything that comes before the
    // ':' for a single name/value pair.
    void PrintFieldName(const Message& message,
                        const Reflection* reflection,
                        const FieldDescriptor* field,
                        PythonGenerator& generator);

    // Outputs a textual representation of the value of the field supplied on
    // the message supplied or the default value if not set.
    void PrintFieldValue(const Message& message,
                         const Reflection* reflection,
                         const FieldDescriptor* field,
                         int index,
                         PythonGenerator& generator);

    // Print the fields in an UnknownFieldSet.  They are printed by tag number
    // only.  Embedded messages are heuristically identified by attempting to
    // parse them.
    void PrintUnknownFields(const UnknownFieldSet& unknown_fields,
                            PythonGenerator& generator);

    int initial_indent_level_;

    bool single_line_mode_;

    bool use_short_repeated_primitives_;

    bool utf8_string_escaping_;
  };

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(PythonFormat);
};

}  // namespace protobuf

}  // namespace google
#endif  // GOOGLE_PROTOBUF_PYTHON_FORMAT_H__
