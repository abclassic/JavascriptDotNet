////////////////////////////////////////////////////////////////////////////////////////////////////
// File: JavascriptObject.cpp
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

#include "JavascriptObject.h"

#include "JavascriptInterop.h"

#include <stdio.h>
#include <string>

#include "JavascriptContext.h";

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptObject::JavascriptObject(const Handle<v8::Object>& iObject) //, Noesis::Javascript::JavascriptContext^ context)
{
	mObject = new Persistent<v8::Object>(Persistent<v8::Object>::New(iObject));

	this->mContext = JavascriptContext::GetCurrent();

	if (mContext == nullptr) {
		throw gcnew System::InvalidOperationException(gcnew System::String("JavascriptObject: current context needed"));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptObject::~JavascriptObject()
{
	mObject->Dispose();
	delete mObject;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::Object^
JavascriptObject::GetProperty(System::Object^ iKey)
{
	HandleScope newScope;
	Handle<Value> key = JavascriptInterop::ConvertToV8(iKey);
	Local<String> keyStr = key->ToString();
	Handle<Value> value = (*mObject)->Get(key);
	return JavascriptInterop::ConvertFromV8(value);
}

array<System::Object^>^ 
JavascriptObject::GetPropertyNames()
{
	HandleScope newScope;
	JSScope newJsScope(mContext);

	Local<v8::Object> local = Local<v8::Object>::New(*mObject);

	v8::Local<v8::Array> propertyNames = local->GetPropertyNames(); // (*mObject)->GetPropertyNames();
	array<System::Object^>^ result = gcnew array<System::Object^>(propertyNames->Length());
	for(int i = 0; i < result->Length; i++)
	{
		Handle<Value> value = propertyNames->Get(i);
		result[i] = JavascriptInterop::ConvertFromV8(value);
	}
	return result;
}

bool
JavascriptObject::IsEmpty() {
	return mObject->IsEmpty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////