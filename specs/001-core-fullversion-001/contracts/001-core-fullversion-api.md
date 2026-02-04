# API Sketch: 001-core-fullversion-001

本文件为 `specs/_contracts/001-core-public-api.md` 中「API 雏形」的本 feature 占位；正式内容以 plan.md 末尾「契约更新」为准，写回契约后与本文件保持一致。

## 1. Memory

Allocator 接口、Alloc/Free、DefaultAllocator、AlignedAlloc、PoolAllocator（可选）、DebugAllocator/LeakTracker（可选）。语义见契约能力 1。

## 2. Thread

Thread、TLS、Atomic、Mutex、ConditionVariable、TaskQueue 骨架。语义见契约能力 2。

## 3. Platform

FileRead/Write、DirectoryEnumerate、Time/HighResolutionTimer、GetEnv、PathNormalize、平台宏。语义见契约能力 3。

## 4. Log

LogLevel、LogSink、Log、Assert、CrashHandler。语义见契约能力 4。

## 5. Math

Vector2/3/4、Matrix3/4、Quaternion、AABB、Ray、Lerp 等。语义见契约能力 5。

## 6. Containers

Array、Map、String、UniquePtr、SharedPtr；可与分配器配合。语义见契约能力 6。

## 7. ModuleLoad

LoadLibrary、UnloadLibrary、GetSymbol、ModuleInit/Shutdown 回调。语义见契约能力 7。
