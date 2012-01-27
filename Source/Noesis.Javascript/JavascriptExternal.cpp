////////////////////////////////////////////////////////////////////////////////////////////////////
// File: JavascriptExternal.cpp
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

#include "JavascriptExternal.h"

#include "JavascriptContext.h"
#include "JavascriptInterop.h"
#include "JavascriptException.h"
#include "SystemInterop.h"

#include <stdio.h>
#include <string>

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace System::Reflection;

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptExternal::JavascriptExternal(System::Object^ iObject)
{
	mObjectHandle = System::Runtime::InteropServices::GCHandle::Alloc(iObject);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptExternal::~JavascriptExternal()
{
	Clear();
	mObjectHandle.Free();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JavascriptExternal::Clear()
{
	map<string, Persistent<Function> >::iterator methodIterator;

	// clear methods
	for (methodIterator = mMethods.begin(); methodIterator != mMethods.end(); methodIterator++)
		methodIterator->second.Dispose();
	mMethods.clear();

	// clear properties
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::Object^
JavascriptExternal::GetObject()
{
	return mObjectHandle.Target;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Handle<Function>
JavascriptExternal::GetMethod(Handle<String> iName)
{
	string name = *String::Utf8Value(iName);
	map<string, Persistent<Function> >::iterator it;

	it = mMethods.find(name); 
	if (it != mMethods.end())
		return it->second;
	else
	{
		System::Object^ self = mObjectHandle.Target;
		System::Type^ type = self->GetType();
		System::String^ memberName = gcnew System::String(name.c_str());
		cli::array<MemberInfo^>^ members = type->GetMember(memberName);

		if (members->Length > 0 && members[0]->MemberType == MemberTypes::Method)
		{
			JavascriptContext^ context = JavascriptContext::GetCurrent();
			Handle<External> external = External::New(context->WrapObject(members));
			Handle<FunctionTemplate> functionTemplate = FunctionTemplate::New(JavascriptInterop::Invoker, external);
			Handle<Function> function = functionTemplate->GetFunction();

			mMethods[name] = Persistent<Function>::New(function);

			return function;
		}
	}

	return Handle<Function>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Handle<Value>
JavascriptExternal::GetProperty(Handle<String> iName)
{
	string name = *String::Utf8Value(iName);
	System::Object^ self = mObjectHandle.Target;
	System::Type^ type = self->GetType();
	PropertyInfo^ propertyInfo = type->GetProperty(gcnew System::String(name.c_str()));

	if (propertyInfo != nullptr)
	{
		try
		{
			return JavascriptInterop::ConvertToV8(propertyInfo->GetValue(self, nullptr));
		}
		catch(System::Exception^ Exception)
		{
			v8::ThrowException(JavascriptInterop::ConvertToV8(Exception->ToString()));
		}
	}

	return Handle<Value>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Handle<Value>
JavascriptExternal::GetProperty(uint32_t iIndex)
{
	System::Object^ self = mObjectHandle.Target;
	System::Type^ type = self->GetType();
	cli::array<PropertyInfo^>^ propertyInfo = type->GetProperties();
	int index = iIndex;

	// Check if it an array
	if(type->IsArray)
	{
		System::Array^ objectArray = (System::Array^)self;
		return JavascriptInterop::ConvertToV8(objectArray->GetValue(index));
	} 
	
	// Check if it an object
	if(type->IsClass)
	{
		try
		{
			cli::array<int^>^ args = gcnew cli::array<int^>(1);
			args[0] = index;
			System::Object^ object = type->InvokeMember("Item", System::Reflection::BindingFlags::GetProperty, nullptr, self, args,  nullptr);
			return JavascriptInterop::ConvertToV8(object);
		}
		catch(System::Exception^ Exception)
		{
			v8::ThrowException(JavascriptInterop::ConvertToV8(Exception->ToString()));
		}
	}

	// No array or indexer, return null and throw an exception
	return Handle<Value>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Handle<Value>
JavascriptExternal::SetProperty(Handle<String> iName, Handle<Value> iValue)
{
	string name = *String::Utf8Value(iName);
	System::Object^ self = mObjectHandle.Target;
	System::Type^ type = self->GetType();
	PropertyInfo^ propertyInfo = type->GetProperty(gcnew System::String(name.c_str()));

	if (propertyInfo != nullptr){
		try
		{
			propertyInfo->SetValue(self, JavascriptInterop::ConvertFromV8(iValue), nullptr);
			return JavascriptInterop::ConvertToV8(propertyInfo->GetValue(self, nullptr));
		}
		catch(System::Exception^ Exception)
		{
			v8::ThrowException(JavascriptInterop::ConvertToV8(Exception->ToString()));
		}
	}

	return Handle<Value>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Handle<Value>
JavascriptExternal::SetProperty(uint32_t iIndex, Handle<Value> iValue)
{
	System::Object^ self = mObjectHandle.Target;
	System::Type^ type = self->GetType();
	cli::array<PropertyInfo^>^ propertyInfo = type->GetProperties();
	int index = iIndex;

	// Check if it an array or an indexer
	if(type->IsArray)
	{
		System::Array^ objectArray = (System::Array^)self;
		objectArray->SetValue(JavascriptInterop::ConvertFromV8(iValue), index);
		return JavascriptInterop::ConvertToV8(objectArray->GetValue(index));
	} 
	else
	{
		try
		{
			cli::array<int^>^ args = gcnew cli::array<int^>(2);
			args[0] = index;
			args[1] = (int)JavascriptInterop::ConvertFromV8(iValue);
			
			System::Object^ object = type->InvokeMember("Item", System::Reflection::BindingFlags::SetProperty, nullptr, self, args,  nullptr);
			System::Int32^ objectInt = (System::Int32^)object;
			objectInt = (System::Int32^)JavascriptInterop::ConvertFromV8(iValue);
			return JavascriptInterop::ConvertToV8(objectInt);
		}
		catch(System::Exception^ Exception)
		{
			v8::ThrowException(JavascriptInterop::ConvertToV8(Exception->ToString()));
		}
	}

	return Handle<Value>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////