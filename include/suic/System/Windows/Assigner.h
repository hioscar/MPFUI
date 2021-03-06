// 华勤科技版权所有 2008-2022
// 
// 文件名：Assigner.h
// 功  能：定义核心库的事件分发接口。
// 
// 作  者：MPF开发组
// 时  间：2010-07-02
// 
// ============================================================================

#ifndef _UIASSIGNER_H_
#define _UIASSIGNER_H_

#include <System/Windows/Event.h>
#include <System/Windows/HwndAdapter.h>
#include <System/Windows/AssignerTimer.h>
#include <System/Windows/InvokerPriorityQueue.h>

#include <System/Types/Structure.h>
#include <System/Animation/TimerManager.h>

#include <System/Tools/Array.h>
#include <System/Tools/Locker.h>
#include <System/Tools/CollectionX.h>

namespace suic
{

class Popup;
class AssignerFrame;
typedef shared<AssignerFrame> AssignerFramePtr;

class SUICORE_API BindingWorker
{
public:

    int GetTaskCount() const;
    void AddTask(Object* task);
    void Run();

private:

    Array<WeakRef> _tasks;
};

inline int BindingWorker::GetTaskCount() const
{
    return _tasks.Length();
}

class SUICORE_API Assigner : public Object
{
public:

    Point lastPoint;
    Int32 lastMessage;
    MessageParam lastmp;
    // 当前线程分发器锁
    Mutex insLock;
    bool iscontinue;

    ~Assigner();

public:

    bool CheckAccess();
    void VerifyAccess(char* f, int l);

    /// <summary>
    ///     取得当前的分发器对象，一个线程仅仅有一个
    /// </summary>
    /// <returns>AssignerPtr：分配器对象</returns> 
    static Assigner* Current();

    static MessageParam* Lastmp();

    /// <summary>
    ///     在最后插入一个消息循环
    /// </summary>
    /// <remarks>
    ///     此函数将使用一个AssignerFrame对象来阻塞并分发
    ///     当前窗口消息，当前消息退出后，将进入上一个消息
    ///     分发器
    /// </remarks>
    /// <param name="frame">
    ///     循环执行对象
    /// </param>
    /// <returns>无</returns> 
    static void PushMessageLoop(AssignerFrame* frame);

    BindingWorker* GetBindingWorker();
    TimeManager* GetTimeManager();

    /// <summary>
    ///     创建一个隐藏窗口用于内部消息分发
    /// </summary>
    /// <returns>无</returns> 
    void CreateMessageHwnd();

    void AddTimer(AssignerTimer* timer);
    void RemoveTimer(AssignerTimer* timer);

    int GetAssignerFrameCount();
    AssignerFrame* GetTopAssignerFrame();
    AssignerFrame* GetAssignerFrame(int index);

    /// <summary>
    ///     执行消息循环
    /// </summary>
    /// <remarks>
    ///     一开始新建一个AssignerFrame对象进行消息处理
    /// </remarks>
    /// <returns>无</returns> 
    void Run();

    /// <summary>
    ///     处理真正的消息分发和处理
    /// </summary>
    /// <remarks>
    ///     消息在这里将会分发到指定的窗口
    /// </remarks>
    /// <param name="msg">
    ///     消息对象
    /// </param>
    /// <returns>无</returns> 
    void TranslateAndAssignMessage(MSG * msg);

    /// <summary>
    ///     接收隐藏窗口消息处理队列
    /// </summary>
    /// <param name="hwnd">
    ///     窗口句柄
    /// </param>
    /// <param name="message">
    ///     消息ID
    /// </param>
    /// <returns>执行后返回值</returns> 
    LRESULT WndProcHook(Handle hwnd, Int32 message, Uint32 wParam, Uint32 lParam, bool& handled);

    /// <summary>
    ///     判断是否有输入消息
    /// </summary>
    /// <returns>true: 有输入消息;false: 无输入消息</returns> 
    bool IsInputPending();

    /// <summary>
    ///     执行消息的处理
    /// </summary>
    /// <param name="force">
    ///     强制处理消息
    /// </param>
    /// <returns>true: 消息已处理;false: 放入队列</returns> 
    bool RequestProcessing(bool force=false);

    void ProcessMessageQueue();
    void PromoteTimers(Uint32 currTimeInTicks);

    /// <summary>
    ///     同步处理消息
    /// </summary>
    /// <param name="data">
    ///     执行对象
    /// </param>
    /// <param name="timeout">
    ///     等待超时时间
    /// </param>
    void Invoke(AssignerInvoker* data, Int32 timeout);

    /// <summary>
    ///     执行异步消息处理
    /// </summary>
    /// <remarks>
    ///     此函数不进行等待，调用后将马上返回，不论消息执行完与否
    /// </remarks>
    // <param name="data">
    ///     执行对象
    /// </param>
    /// <returns>无</returns> 
    void BeginInvoke(AssignerInvoker* data);

    Uint32 GetThreadId();

    void RegisterHook(Object* obj, MessageHook hook);
    void RemoveHookFromObject(Object* obj);
    void RemoveHook(Object* obj, MessageHook hook);

    bool DoMessageHook(Object* root, MessageParam* mp, bool& interrupt);
    void RecycleObject(Object* obj);

    Popup* GetTopTrackPopup();

    bool PopTrackPopup();
    void PushTrackPopup(Popup* popup);
    void RemoveTrackPopup(Popup* popup);

    void RemoveAssignerFrame(AssignerFrame* frame);
    void InsertAssignerFrame(int index, AssignerFrame* frame);

protected:

    bool IsIdleMessage(MSG* pMsg);
    void ProcessSenderMessageQueue();

private:

    Assigner();

    void DoInit();
    void DoRealMessageLoop(AssignerFrame* frame);
    void DoRealShutdown();
    void InternalShutdown();

    bool OnIdle(int iIdleCount);

    bool IsWindowNull();

    bool DoForegroundMessage();
    bool DoBackgroundMessage();
    void DoRecyleObjects();

    void SetInternalTimer(int iTicks);
    void KillInternalTimer();

    void UpdateDispachterTimer(bool unUsed);

private:

    // 隐藏窗口的句柄
    Handle _hwnd;
    static Assigner* _assigner;
    TimeManager* _timeManager;

    Int32 _currentDoType;
    Int32 _msgProcessQueue;
    Int32 _msgRecycleObj;
    Int32 _frameDepth;
    bool _exitAllFrames;
    bool _hasFinishedClose;
    bool _hasStartedClose;
    bool _startingClose;
    Uint32 _threadId;
    Uint32 _timersVersion;
    Uint32 _dueTimeInTicks;

    bool _isTimerSet;
    bool _dueTimeFound;

    BindingWorker* _bindWorker;
    InvokerPriorityQueue* _queue;
    
    HookInfo _hooks;

    Array<Popup*> _trackPopups;
    Array<Object*> _recyEles;
    Array<AssignerFrame*> _frames;
    Array<AssignerTimer*> _timers;

    friend class AssignerFrame;
};

inline Uint32 Assigner::GetThreadId()
{
    return _threadId;
}

inline BindingWorker* Assigner::GetBindingWorker()
{
    return _bindWorker;
}

class SUICORE_API AssignerFrame : public Object
{
public:

    bool _continue;
    bool _exitWhenRequested;

    AssignerFrame(bool exitWhenRequested);
    AssignerFrame();

    ~AssignerFrame();

    Assigner* GetAssigner();
    bool IsContinue();
    void SetContinue(bool value);

    void Clear();

    void RegisterHook(Object* obj, MessageHook hook);
    void RemoveHookFromObject(Object* obj);
    void RemoveHook(Object* obj, MessageHook hook);

    void ClearHooks();

protected:

    Assigner* _assigner;
    HookInfo _hooks;

    friend class HwndSubclass;
};

};

#endif

