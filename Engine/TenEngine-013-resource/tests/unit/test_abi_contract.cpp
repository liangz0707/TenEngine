/**
 * @file test_abi_contract.cpp
 * @brief ABI contract consistency test: include paths and symbols must match
 *        specs/_contracts/001-core-ABI.md. Compile + link = contract satisfied.
 */
#include "te/core/alloc.h"
#include "te/core/check.h"
#include "te/core/containers.h"
#include "te/core/engine.h"
#include "te/core/log.h"
#include "te/core/math.h"
#include "te/core/module_load.h"
#include "te/core/platform.h"
#include "te/core/thread.h"

#include <cassert>
#include <functional>

using namespace te::core;

static void noop_init() {}
static void noop_shutdown() {}

int main() {
  // --- alloc.h (te/core/alloc.h) ---
  void* p = Alloc(8, 8);
  assert(p == nullptr || true);
  Free(p);
  Free(nullptr);
  Allocator* def = GetDefaultAllocator();
  (void)def;
  DefaultAllocator default_alloc;
  (void)default_alloc;

  // --- engine.h ---
  InitParams params{};
  bool ok = Init(&params);
  (void)ok;
  Shutdown();

  // --- thread.h (type + APIs only; Thread pimpl requires te_core impl TU for dtor) ---
  (void)sizeof(Thread);
  TLS<int> tls;
  tls.Set(1);
  (void)tls.Get();
  Atomic<int> a(0);
  a.Store(1);
  (void)a.Load();
  Mutex m;
  LockGuard g(m);
  ConditionVariable cv;
  (void)cv;
  TaskQueue q;
  (void)q;
  IThreadPool* pool = GetThreadPool();
  (void)pool;

  // --- platform.h ---
#if TE_PLATFORM_WINDOWS + TE_PLATFORM_LINUX + TE_PLATFORM_MACOS + TE_PLATFORM_ANDROID + TE_PLATFORM_IOS >= 1
  (void)0;
#endif
  (void)FileRead(std::string{});
  (void)FileWrite(std::string{}, std::vector<std::uint8_t>{});
  (void)FileWrite(std::string{}, std::string{});
  DirEntry e;
  (void)e;
  (void)DirectoryEnumerate(std::string{});
  (void)Time();
  (void)HighResolutionTimer();
  (void)GetEnv("");
  (void)PathNormalize("");

  // --- log.h ---
  Log(LogLevel::Debug, "abi test");
  LogSetLevelFilter(LogLevel::Info);
  LogSetStderrThreshold(LogLevel::Warn);
  LogSetSink(nullptr);
  SetCrashHandler(nullptr);
  (void)CrashHandlerFn{};

  // --- check.h (macros) ---
  CheckWarning(true);
  CheckWarning(true, "msg");
  CheckError(true);
  CheckError(true, "msg");

  // --- math.h ---
  (void)Scalar(0.f);
  Vector2 v2{};
  Vector3 v3{};
  Vector4 v4{};
  (void)Lerp(0.f, 1.f, 0.5f);
  (void)Lerp(v2, v2, 0.5f);
  (void)Dot(v2, v2);
  (void)Cross(v3, v3);
  (void)Length(v2);
  (void)Normalize(v2);
  Matrix3 m3{};
  Matrix4 m4{};
  Quaternion qq{};
  AABB box{};
  Ray ray{};
  (void)m3;
  (void)m4;
  (void)qq;
  (void)box;
  (void)ray;

  // --- containers.h ---
  Array<int> arr;
  Map<int, int> map;
  String s;
  UniquePtr<int> up;
  SharedPtr<int> sp;
  (void)arr;
  (void)map;
  (void)s;
  (void)up;
  (void)sp;

  // --- module_load.h ---
  (void)ModuleHandle{nullptr};
  (void)LoadLibrary("");
  UnloadLibrary(nullptr);
  (void)GetSymbol(nullptr, "");
  RegisterModuleInit(noop_init);
  RegisterModuleShutdown(noop_shutdown);
  RunModuleInit();
  RunModuleShutdown();

  return 0;
}
