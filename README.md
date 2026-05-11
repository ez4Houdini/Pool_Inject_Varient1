  第一个模板，反检测：采用了TLS回调检测Debug（检测虚拟机逻辑没写），然后用COM调用WMI查询CPU核心反虚拟机。
  采用PoolParty的第一种方式TpWorkerFactory进行线程池注入，用未文档化的Nt函数修改线程池的回调，具体先是拿到句柄表->枚举句柄->劫持句柄 -> 跨进程Dump到本进程 -> 查询对应的TpWorkerFactory句柄 ->修改StartRoutine到shellcode ->VirtualAlloc&&WriteProcessMemory
  经测试，程序逻辑一切正常。
  关于shellcode:第一段xor加密解密的x86 shellcode是自己动态定位然后加载同目录下mydll.dll里的loader()函数，地址都是动态的,第二段是弹出计算器的x64shellcode，可以做进一步花指令混淆
  未来可配合BYOVD驱动，传IOCTL来达成想要的目的，目前可能是因为恶意逻辑较少，VT中EDR中检测率偏低，主流杀软仅有卡巴斯基，AVAST可以发现，注入器可以配合DLL的反射加载更隐蔽
  未文档化的Nt函数，使用 C + WinAPI 重构部分未文档化 Nt API 调用逻辑。
  代码尚未完成(todo注入后操作)，依照目前的趋势看来，线程池注入还是非常隐蔽且是最优的方式，相比于APC和远程线程,EDR Hook的概率大幅度降低,记得加入ByPassUAC,本文档不附带
  参考技术：COM组件，PoolParty项目，TLS回调技术，WIndows ThreadPool相关逻辑，微软MSDN文档
