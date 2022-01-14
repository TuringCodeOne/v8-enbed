#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libplatform/libplatform.h"
#include "v8-context.h"
#include "v8-initialization.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-primitive.h"
#include "v8-script.h"
#include "v8.h"

using namespace v8;

Local<Context> CreateShellContext(Isolate *isolate);
void Print(const v8::FunctionCallbackInfo<v8::Value> &args);

// Extracts a C string from a V8 Utf8Value.
const char *ToCString(const v8::String::Utf8Value &value)
{
  return *value ? *value : "<string conversion failed>";
}

int main(int argc, char *argv[])
{
  // Initialize V8.
  V8::InitializeICUDefaultLocation(argv[0]);
  /**
   * 用于设置启动快照数据路径接口，在v8启动的时候设置。V8在启动的时候会需要预先加载一些build-in函数。V8引入了快照加载功能用于加速这个过程, 目前依赖两种快照natives_blob.bin和snapshot_blob.bin。 所以InitializeExternalStartupData函数就是设置V8的保存快照的路径。

    natives_blob.bin文件：存储了build-in函数解析执行后的内存布局快照。build-in可以在v8/src/objects.h中的FUNCTIONS_WITH_ID_LIST宏中查看。

    snapshot_blob.bin: 存储了V8启动后的内存布局快照。
  */
  V8::InitializeExternalStartupData(argv[0]);

  // 平台初始化
  std::unique_ptr<Platform> platform = platform::NewDefaultPlatform();
  V8::InitializePlatform(platform.get());
  V8::Initialize();

  // 创建一个v8实例
  Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      ArrayBuffer::Allocator::NewDefaultAllocator();
  // 创建实例
  Isolate *isolate = Isolate::New(create_params);
  // 进入实例
  Isolate::Scope isolate_scope(isolate);
  // 创建handleScope
  HandleScope scope(isolate);

  Local<Context> context = CreateShellContext(isolate);

  Context::Scope contextScope(context);

  // 下面可以创建各种handle了
  Local<String> source =
      String::NewFromUtf8Literal(isolate, "print(1, (param)=> {print(param)});");

  Local<Script> script = Script::Compile(context, source).ToLocalChecked();
  v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

  // Convert the result to an UTF8 string and print it.
  v8::String::Utf8Value utf8(isolate, result);
  printf("%s\n", *utf8);
}

Local<Context> CreateShellContext(Isolate *isolate)
{
  // Create a template for the global object.
  Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
  // Bind the global 'print' function to the C++ Print callback.
  global->Set(isolate, "print", FunctionTemplate::New(isolate, Print));
  return Context::New(isolate, NULL, global);
}

void Print(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  String::Utf8Value str(isolate, args[0]);
  const char *cstr = ToCString(str);
  printf("%s", cstr);

  if (args[1]->IsFunction())
  {
    Local<Function> callback = Local<Function>::Cast(args[1]);

    const unsigned argc = 1;

    Local<Value> argv[argc] = { Number::New(isolate, 520) };
    Local<Context> context = Context::New(isolate);

    callback->Call(context, Null(isolate), argc, argv);
  }
}
