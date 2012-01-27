////////////////////////////////////////////////////////////////////////////////////////////////////
// File: JavascriptException.cpp
// 
// Copyright 2010 Noesis Innovation Inc. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
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
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "JavascriptException.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace v8;

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptException::JavascriptException(TryCatch& iTryCatch): System::Exception()
{
	v8::Handle<v8::Message> message = iTryCatch.Message();

	mMessage = gcnew System::String(*String::Utf8Value(iTryCatch.Exception()));

	if (!message.IsEmpty())
	{
		mSource = gcnew System::String(*String::Utf8Value(message->GetScriptResourceName()));
		mLine = message->GetLineNumber();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::String^
JavascriptException::Message::get()
{
	return mMessage;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::String^
JavascriptException::Source::get()
{
	return mSource;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
JavascriptException::Line::get()
{
	return mLine;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////