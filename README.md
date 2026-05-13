  Warning!!!：本代码仅用于教育用途，可能会损害你的机器，请在虚拟机下运行并在24小时内删除，且运行后果自负，下载本代码视为同意该协议<br><br>
  第一个模板，反检测：采用了TLS回调检测Debug（检测虚拟机逻辑没写），然后用COM调用WMI查询CPU核心反虚拟机。<br><br>
  采用PoolParty的第一种方式TpWorkerFactory进行线程池注入，用未文档化的Nt函数修改线程池的回调，具体先是拿到句柄表->枚举句柄->劫持句柄 -> 跨进程Dump到本进程 -> 查询对应的TpWorkerFactory句柄 ->修改StartRoutine到shellcode ->VirtualAlloc&&WriteProcessMemory
  经测试，程序逻辑一切正常。<br><br>
  关于shellcode:第一段xor加密解密的x86 shellcode是自己动态定位然后加载同目录下mydll.dll里的loader()函数，地址都是动态的,第二段是弹出计算器的x64shellcode，可以做进一步花指令混淆<br><br>
  未来可配合BYOVD驱动，传IOCTL来达成想要的目的，目前可能是因为恶意逻辑较少，VT中EDR中检测率偏低，主流杀软仅有卡巴斯基，AVAST可以发现，注入器可以配合DLL的反射加载更隐蔽<br><br>
  未文档化的Nt函数，使用 C + WinAPI 重构部分未文档化 Nt API 调用逻辑。<br><br>
  代码尚未完成(todo注入后操作)，依照目前的趋势看来，线程池注入还是非常隐蔽且是最优的方式，相比于APC和远程线程,EDR Hook的概率大幅度降低,记得加入ByPassUAC,本文档不附带<br><br>
  参考技术：COM组件，PoolParty项目，TLS回调技术，WIndows ThreadPool相关逻辑，微软MSDN文档<br><br>

  =======================================================================================================<br><br>
  2026年5月12日00点01分 <br><br>
  --添加BYOVD相关逻辑（并未完善）漏洞驱动sha256:2fdfdd13a0c548bb68c9d5aa8599a9265d4659da3e237fe7a42ac6ac06b9a06a,可以在LOLDrivers找到，传IOCTL：0x12227A可以不鉴权执行ZwTerminateProcess<br<br>
  --加入AV进程名数组，可以改进传数组循环Kill<br><br>
  --需要完善loader善后逻辑和多线程并行，可以分批drop Payload<br><br>
  =======================================================================================================<br><br>
  2026年5月13日22点43分 <br><br>
  --增加了最基础的 控制流平坦化 及 非透明谓词 混淆<br><br>
  --增加了TP_IO投递线程池的注入方式，优化了逻辑<br><br>
  --恶意度增加，vt检出率21/69，大多是国外的EDR<br><br>
  
  
  
