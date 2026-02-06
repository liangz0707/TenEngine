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

// Module init/shutdown callbacks for testing
static void noop_init() {}
static void noop_shutdown() {}

// Task callback for testing thread pool
static void noop_task(void*) {}

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
  // Test new enhanced functions
  void* p2 = AllocAligned(16, 16);
  Free(p2);
  void* p3 = Realloc(nullptr, 32);
  Free(p3);
  MemoryStats stats = GetMemoryStats();
  (void)stats.allocated_bytes;
  (void)stats.peak_bytes;
  (void)stats.allocation_count;

  // --- engine.h ---
  InitParams params{};
  bool ok = Init(&params);
  (void)ok;
  Shutdown();

  // --- thread.h ---
  Thread t;
  (void)t;
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
  // Test new enhanced thread pool functions
  TaskId taskId = pool->SubmitTaskWithPriority(noop_task, nullptr, 1);
  (void)taskId;
  bool cancelled = pool->CancelTask(taskId);
  (void)cancelled;
  TaskStatus status = pool->GetTaskStatus(taskId);
  (void)status;
  pool->SetCallbackThread(CallbackThreadType::MainThread);

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
  // Test new enhanced file I/O functions
  char buffer[1024];
  std::size_t readSize = 0;
  (void)FileReadBinary("", buffer, &readSize, 0, 1024);
  (void)FileWriteBinary("", buffer, 0, 0);
  (void)FileGetSize("");
  (void)FileExists("");
  // Test new enhanced path functions
  (void)PathJoin("", "");
  (void)PathGetDirectory("");
  (void)PathGetFileName("");
  (void)PathGetExtension("");
  (void)PathResolveRelative("", "");

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
