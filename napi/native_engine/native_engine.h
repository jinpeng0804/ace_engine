/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_ENGINE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_ENGINE_H

#include <functional>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#ifdef LINUX_PLATFORM
#include<atomic>
#endif

#include "callback_scope_manager/native_callback_scope_manager.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "module_manager/native_module_manager.h"
#include "native_engine/native_async_work.h"
#include "native_engine/native_deferred.h"
#include "native_engine/native_reference.h"
#include "native_engine/native_safe_async_work.h"
#include "native_engine/native_value.h"
#include "native_property.h"
#include "reference_manager/native_reference_manager.h"
#include "utils/macros.h"

namespace panda::ecmascript {
    class EcmaVM;
}

typedef int32_t (*GetContainerScopeIdCallback)(void);
typedef void (*ContainerScopeCallback)(int32_t);
typedef struct uv_loop_s uv_loop_t;

struct NativeErrorExtendedInfo {
    const char* message = nullptr;
    void* reserved = nullptr;
    uint32_t engineErrorCode = 0;
    int errorCode = 0;
};

struct ExceptionInfo {
    const char* message_ = nullptr;
    int32_t lineno_ = 0;
    int32_t colno_ = 0;

    ~ExceptionInfo()
    {
        if (message_ != nullptr) {
            delete[] message_;
        }
    }
};

enum LoopMode {
    LOOP_DEFAULT, LOOP_ONCE, LOOP_NOWAIT
};

enum class DumpFormat {
    JSON, BINARY, OTHER
};

enum class WorkerVersion {
    NONE, OLD, NEW
};

class CleanupHookCallback {
public:
    using Callback = void (*)(void*);

    CleanupHookCallback(Callback fn, void* arg, uint64_t insertion_order_counter)
        : fn_(fn), arg_(arg), insertion_order_counter_(insertion_order_counter)
    {}

    struct Hash {
        inline size_t operator()(const CleanupHookCallback& cb) const
        {
            return std::hash<void*>()(cb.arg_);
        }
    };
    struct Equal {
        inline bool operator()(const CleanupHookCallback& a, const CleanupHookCallback& b) const
        {
            return a.fn_ == b.fn_ && a.arg_ == b.arg_;
        };
    };

private:
    friend class NativeEngine;
    Callback fn_;
    void* arg_;
    uint64_t insertion_order_counter_;
};

using PostTask = std::function<void(bool needSync)>;
using CleanEnv = std::function<void()>;
using InitWorkerFunc = std::function<void(NativeEngine* engine)>;
using GetAssetFunc = std::function<void(const std::string& uri, std::vector<uint8_t>& content, std::string& ami)>;
using OffWorkerFunc = std::function<void(NativeEngine* engine)>;
using DebuggerPostTask = std::function<void(std::function<void()>&&)>;
using NapiUncaughtExceptionCallback = std::function<void(napi_value value)>;
using PermissionCheckCallback = std::function<bool()>;
using NapiConcurrentCallback = void (*)(napi_env env, napi_value result, bool success, void* data);
using SourceMapCallback = std::function<std::string(const std::string& rawStack)>;
using SourceMapTranslateCallback = std::function<bool(std::string& url, int& line, int& column)>;
using EcmaVM = panda::ecmascript::EcmaVM;

class NAPI_EXPORT NativeEngine {
public:
    explicit NativeEngine(void* jsEngine);
    virtual ~NativeEngine();

    virtual NativeModuleManager* GetModuleManager();
    virtual NativeReferenceManager* GetReferenceManager();
    virtual NativeCallbackScopeManager* GetCallbackScopeManager();
    virtual uv_loop_t* GetUVLoop() const;
    virtual pthread_t GetTid() const;

    virtual bool ReinitUVLoop();

    virtual void Loop(LoopMode mode, bool needSync = false);
    virtual void SetPostTask(PostTask postTask);
    virtual void TriggerPostTask();
#if !defined(PREVIEW)
    virtual void CheckUVLoop();
    virtual void CancelCheckUVLoop();
#endif
    virtual void* GetJsEngine();

    virtual const EcmaVM* GetEcmaVm() const = 0;
    virtual void SetPromiseRejectCallback(NativeReference* rejectCallbackRef, NativeReference* checkCallbackRef) = 0;

    virtual bool InitTaskPoolThread(NativeEngine* engine, NapiConcurrentCallback callback) = 0;
    virtual bool InitTaskPoolThread(napi_env env, NapiConcurrentCallback callback) = 0;
    virtual bool InitTaskPoolFunc(napi_env env, napi_value func, void* taskInfo) = 0;
    virtual bool HasPendingJob() const = 0;
    virtual bool IsProfiling() const = 0;
    virtual bool IsExecutingPendingJob() const = 0;
    virtual void* GetCurrentTaskInfo() const = 0;
    virtual void TerminateExecution() const = 0;

    virtual napi_value CallFunction(napi_value thisVar,
                                    napi_value function,
                                    napi_value const *argv,
                                    size_t argc) = 0;
    virtual bool RunScriptPath(const char* path) = 0;
    virtual napi_value RunScriptBuffer(const char* path, std::vector<uint8_t>& buffer, bool isBundle) = 0;
    virtual bool RunScriptBuffer(const std::string &path, uint8_t* buffer, size_t size, bool isBundle) = 0;
    virtual napi_value RunBufferScript(std::vector<uint8_t>& buffer) = 0;
    virtual napi_value RunActor(std::vector<uint8_t>& buffer, const char* descriptor, char* entryPoint = nullptr) = 0;

    virtual napi_value CreateInstance(napi_value constructor, napi_value const *argv, size_t argc) = 0;

    virtual NativeReference* CreateReference(napi_value value, uint32_t initialRefcount,
        bool flag = false, NapiNativeFinalize callback = nullptr, void* data = nullptr, void* hint = nullptr) = 0;

    virtual NativeAsyncWork* CreateAsyncWork(napi_value asyncResource,
                                             napi_value asyncResourceName,
                                             NativeAsyncExecuteCallback execute,
                                             NativeAsyncCompleteCallback complete,
                                             void* data);

    virtual NativeAsyncWork* CreateAsyncWork(const std::string &asyncResourceName,
                                             NativeAsyncExecuteCallback execute,
                                             NativeAsyncCompleteCallback complete,
                                             void* data);
    virtual NativeSafeAsyncWork* CreateSafeAsyncWork(napi_value func, napi_value asyncResource,
        napi_value asyncResourceName, size_t maxQueueSize, size_t threadCount, void* finalizeData,
        NativeFinalize finalizeCallback, void* context, NativeThreadSafeFunctionCallJs callJsCallback);

    virtual void* CreateRuntime(bool isLimitedWorker = false) = 0;
    virtual napi_value CreatePromise(NativeDeferred** deferred) = 0;

    virtual void StartCpuProfiler(const std::string& fileName = "") = 0;
    virtual void StopCpuProfiler() = 0;

    virtual void ResumeVM() = 0;
    virtual bool SuspendVM() = 0;
    virtual bool IsSuspended() = 0;
    virtual bool CheckSafepoint() = 0;
    virtual bool SuspendVMById(uint32_t tid) = 0;
    virtual void ResumeVMById(uint32_t tid) = 0;

    virtual void DumpHeapSnapshot(const std::string &path, bool isVmMode = true,
        DumpFormat dumpFormat = DumpFormat::JSON) = 0;
    virtual void DumpHeapSnapshot(bool isVmMode = true, DumpFormat dumpFormat = DumpFormat::JSON,
        bool isPrivate = false, bool isFullGC = true) = 0;
    virtual bool BuildNativeAndJsStackTrace(std::string &stackTraceStr) = 0;
    virtual bool BuildJsStackTrace(std::string &stackTraceStr) = 0;
    virtual bool BuildJsStackInfoList(uint32_t tid, std::vector<JsFrameInfo>& jsFrames) = 0;
    virtual bool DeleteWorker(NativeEngine* workerEngine) = 0;
    virtual bool StartHeapTracking(double timeInterval, bool isVmMode = true) = 0;
    virtual bool StopHeapTracking(const std::string &filePath) = 0;

    virtual void AllowCrossThreadExecution() const = 0;

    NativeErrorExtendedInfo* GetLastError();
    void SetLastError(int errorCode, uint32_t engineErrorCode = 0, void* engineReserved = nullptr);
    void ClearLastError()
    {
        lastError_.errorCode = 0;
        lastError_.engineErrorCode = 0;
        lastError_.message = nullptr;
        lastError_.reserved = nullptr;
    }
    void EncodeToUtf8(napi_value value, char* buffer, int32_t* written, size_t bufferSize, int32_t* nchars);
    void EncodeToChinese(napi_value value, std::string& buffer, const std::string& encoding);
    NativeEngine(NativeEngine&) = delete;
    virtual NativeEngine& operator=(NativeEngine&) = delete;

    virtual napi_value ValueToNapiValue(JSValueWrapper& value) = 0;
    virtual std::string GetSourceCodeInfo(napi_value value, ErrorPos pos) = 0;

    virtual bool TriggerFatalException(napi_value error) = 0;
    virtual bool AdjustExternalMemory(int64_t ChangeInBytes, int64_t* AdjustedValue) = 0;

    void MarkWorkerThread()
    {
        jsThreadType_ = JSThreadType::WORKER_THREAD;
    }
    void MarkRestrictedWorkerThread()
    {
        jsThreadType_ = JSThreadType::RESTRICTEDWORKER_THREAD;
    }
    void MarkTaskPoolThread()
    {
        jsThreadType_ = JSThreadType::TASKPOOL_THREAD;
    }
    bool IsWorkerThread() const
    {
        return jsThreadType_ == JSThreadType::WORKER_THREAD;
    }
    bool IsRestrictedWorkerThread() const
    {
        return jsThreadType_ == JSThreadType::RESTRICTEDWORKER_THREAD;
    }
    bool IsTaskPoolThread() const
    {
        return jsThreadType_ == JSThreadType::TASKPOOL_THREAD;
    }
    bool IsMainThread() const
    {
        return jsThreadType_ == JSThreadType::MAIN_THREAD;
    }

    bool CheckAndSetWorkerVersion(WorkerVersion expected, WorkerVersion desired)
    {
        return workerVersion_.compare_exchange_strong(expected, desired);
    }
    bool IsTargetWorkerVersion(WorkerVersion target) const
    {
        return workerVersion_.load() == target;
    }

    void IncreaseSubEnvCounter()
    {
        subEnvCounter_++;
    }
    void DecreaseSubEnvCounter()
    {
        subEnvCounter_--;
    }
    bool HasSubEnv()
    {
        return subEnvCounter_.load() != 0;
    }

    void SetCleanEnv(CleanEnv cleanEnv)
    {
        cleanEnv_ = cleanEnv;
    }

    // register init worker func
    virtual void SetInitWorkerFunc(InitWorkerFunc func);
    InitWorkerFunc GetInitWorkerFunc() const;
    virtual void SetGetAssetFunc(GetAssetFunc func);
    GetAssetFunc GetGetAssetFunc() const;
    virtual void SetOffWorkerFunc(OffWorkerFunc func);
    OffWorkerFunc GetOffWorkerFunc() const;

    // call init worker func
    virtual bool CallInitWorkerFunc(NativeEngine* engine);
    virtual bool CallGetAssetFunc(const std::string& uri, std::vector<uint8_t>& content, std::string& ami);
    virtual bool CallOffWorkerFunc(NativeEngine* engine);

    // adapt worker to ace container
    virtual void SetGetContainerScopeIdFunc(GetContainerScopeIdCallback func);
    virtual void SetInitContainerScopeFunc(ContainerScopeCallback func);
    virtual void SetFinishContainerScopeFunc(ContainerScopeCallback func);
    virtual int32_t GetContainerScopeIdFunc();
    virtual bool InitContainerScopeFunc(int32_t id);
    virtual bool FinishContainerScopeFunc(int32_t id);

#if !defined(PREVIEW)
    virtual void SetDebuggerPostTaskFunc(DebuggerPostTask func);
    virtual void CallDebuggerPostTaskFunc(std::function<void()>&& task);
#endif

    virtual void SetHostEngine(NativeEngine* engine);
    virtual NativeEngine* GetHostEngine() const;

    using CleanupCallback = CleanupHookCallback::Callback;
    virtual void AddCleanupHook(CleanupCallback fun, void* arg);
    virtual void RemoveCleanupHook(CleanupCallback fun, void* arg);

    void CleanupHandles();
    void IncreaseWaitingRequestCounter();
    void DecreaseWaitingRequestCounter();
    bool HasWaitingRequest();

    void IncreaseListeningCounter();
    void DecreaseListeningCounter();
    bool HasListeningCounter();

    virtual void RunCleanup();

    bool IsStopping() const
    {
        return isStopping_.load();
    }

    void SetStopping(bool value)
    {
        isStopping_.store(value);
    }

    virtual void PrintStatisticResult() = 0;
    virtual void StartRuntimeStat() = 0;
    virtual void StopRuntimeStat() = 0;
    virtual size_t GetArrayBufferSize() = 0;
    virtual size_t GetHeapTotalSize() = 0;
    virtual size_t GetHeapUsedSize() = 0;
    virtual size_t GetHeapLimitSize() = 0;
    virtual void NotifyApplicationState(bool inBackground) = 0;
    virtual void NotifyIdleStatusControl(std::function<void(bool)> callback) = 0;
    virtual void NotifyIdleTime(int idleMicroSec) = 0;
    virtual void NotifyMemoryPressure(bool inHighMemoryPressure = false) = 0;
    virtual void NotifyForceExpandState(int32_t value) = 0;
    virtual void SetMockModuleList(const std::map<std::string, std::string> &list) = 0;

    void RegisterWorkerFunction(const NativeEngine* engine);
    virtual void RegisterNapiUncaughtExceptionHandler(NapiUncaughtExceptionCallback callback) = 0;
    virtual void HandleUncaughtException() = 0;
    virtual bool HasPendingException()
    {
        return false;
    }
    virtual void RegisterPermissionCheck(PermissionCheckCallback callback) = 0;
    virtual bool ExecutePermissionCheck() = 0;
    virtual void RegisterTranslateBySourceMap(SourceMapCallback callback) = 0;
    virtual std::string ExecuteTranslateBySourceMap(const std::string& rawStack) = 0;
    virtual void RegisterSourceMapTranslateCallback(SourceMapTranslateCallback callback) = 0;
    virtual void SetPromiseRejectCallBackRef(NativeReference*) = 0;
    virtual void SetCheckCallbackRef(NativeReference*) = 0;
    virtual NapiUncaughtExceptionCallback GetNapiUncaughtExceptionCallback() = 0;
    virtual void* GetPromiseRejectCallback() = 0;
    virtual void GetCurrentModuleName(std::string& moduleName) = 0;
    // run script by path
    napi_value RunScript(const char* path, char* entryPoint = nullptr);

    const char* GetModuleFileName();

    void SetModuleName(std::string &moduleName);
    void SetModuleFileName(std::string &moduleFileName);

    void SetInstanceData(void* data, NativeFinalize finalize_cb, void* hint);
    void GetInstanceData(void** data);

    /**
     * @brief Set the Extension Infos
     *
     * @param extensionInfos extension infos to set
     */
    void SetExtensionInfos(std::unordered_map<std::string, int32_t>&& extensionInfos);

    /**
     * @brief Get the Extension Infos
     *
     * @return extension infos
     */
    const std::unordered_map<std::string, int32_t>& GetExtensionInfos();

    /**
     * @brief Set the Module Blocklist
     *
     * @param blocklist the blocklist set to native engine
     */
    void SetModuleBlocklist(std::unordered_map<int32_t, std::unordered_set<std::string>>&& blocklist);

    /**
     * @brief Set the Module Load Checker
     *
     * @param moduleCheckerDelegate the module checker delegate will intercept the module loading
     */
    void SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate>& moduleCheckerDelegate);

    virtual napi_value NapiLoadModule(const char* str) = 0;

private:
    void StartCleanupTimer();
protected:
    void *jsEngine_;
    void* jsEngineInterface_;

    void Init();
    void Deinit();

    NativeModuleManager* moduleManager_ = nullptr;
    NativeReferenceManager* referenceManager_ = nullptr;
    NativeCallbackScopeManager* callbackScopeManager_ = nullptr;

    uv_loop_t* loop_ = nullptr;

    NativeErrorExtendedInfo lastError_;

    // register for worker
    InitWorkerFunc initWorkerFunc_ {nullptr};
    GetAssetFunc getAssetFunc_ {nullptr};
    OffWorkerFunc offWorkerFunc_ {nullptr};
#if !defined(PREVIEW)
    DebuggerPostTask debuggerPostTaskFunc_ {nullptr};
#endif
    NativeEngine* hostEngine_ {nullptr};
    bool isAppModule_ = false;

public:
    uint64_t openHandleScopes_ = 0;
    panda::Local<panda::ObjectRef> lastException_;

private:
    std::string moduleName_;
    std::string moduleFileName_;
    std::mutex instanceDataLock_;
    NativeObjectInfo instanceDataInfo_;
    void FinalizerInstanceData(void);
    pthread_t tid_ = 0;
    std::unordered_map<std::string, int32_t> extensionInfos_;
    uv_sem_t uvSem_;

    // the old worker api use before api9, the new worker api start with api9
    enum JSThreadType { MAIN_THREAD, WORKER_THREAD, TASKPOOL_THREAD, RESTRICTEDWORKER_THREAD};
    JSThreadType jsThreadType_ = JSThreadType::MAIN_THREAD;
    // current is hostengine, can create old worker, new worker, or no workers on hostengine
    std::atomic<WorkerVersion> workerVersion_ { WorkerVersion::NONE };

#if !defined(PREVIEW)
    static void UVThreadRunner(void* nativeEngine);
    void PostLoopTask();

    bool checkUVLoop_ = false;
    uv_thread_t uvThread_;
#endif

    PostTask postTask_ = nullptr;
    CleanEnv cleanEnv_ = nullptr;
    uv_async_t uvAsync_;
    std::unordered_set<CleanupHookCallback, CleanupHookCallback::Hash, CleanupHookCallback::Equal> cleanup_hooks_;
    uint64_t cleanup_hook_counter_ = 0;
    std::atomic_int requestWaiting_ { 0 };
    std::atomic_int listeningCounter_ { 0 };
    std::atomic_int subEnvCounter_ { 0 };
    std::atomic_bool isStopping_ { false };
    bool cleanupTimeout_ = false;
    uv_timer_t timer_;
};

class TryCatch : public panda::TryCatch {
public:
    explicit TryCatch(napi_env env)
        : panda::TryCatch(reinterpret_cast<NativeEngine*>(env)->GetEcmaVm()),
          engine_(reinterpret_cast<NativeEngine*>(env)) {}

    ~TryCatch()
    {
        if (HasCaught()) {
            engine_->lastException_ = GetException();
        }
    }
private:
   NativeEngine* engine_ = nullptr;
};
#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_ENGINE_H */
