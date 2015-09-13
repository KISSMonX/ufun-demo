#ufun_core_test  

目前因为是测试板卡, 加上很多项目组成员不会使用版本管理和 git.
所以先期使用 Coding 来协作演示. Github 其实是一样的操作效果.  
而且 Coding 支持建立私有项目, 所以比较适合咱们.   

**项目视频:**  

* 优酷视频(不是很清晰): [μFun公益开发板项目及版本管理介绍](http://v.youku.com/v_show/id_XOTMwNjUyMDI0.html?from=y1.7-1.2)  

* 百度网盘: [uFun公益开发板项目介绍及版本管理.mp4](http://yun.baidu.com/s/1ntMK6cL?fid=156488652153958)


**提示: 这个网站提供有 iOS 和 Android 手机客户端. 安装后可以随时跟踪项目情况. 比较方便.**


**项目工程 clone 下来并不能立即使用. 工程需要根据自己的情况重新建立. 这里保存的只是源代码. 
工程文件是二进制的, 所以进行版本管理, 没太大意义, 而且每个人使用的软件版本不一致, 配置也不一致,  
所以可以避免每次都要重新 commit 进仓库里.** 
 
 **项目的文件目录结构如下图:** 
 
![1](http://d.pcs.baidu.com/thumbnail/9359ff5aab58275b476b7e44c789964a?fid=2402591948-250528-586297228753154&time=1431874800&rt=sh&sign=FDTAER-DCb740ccc5511e5e8fedcff06b081203-p21zvLtzhSFXRoOG8Eg9%2FPb6b5E%3D&expires=2h&prisign=unkown&chkv=0&chkbd=0&chkpc=&size=c850_u580&quality=100) 
 
   
   
**ARM-MDK 工程文件需要建立在 project 文件夹里, 而且编译时产生的中间件也要放到对应的子目录里. 如下图:**  


![2](http://d.pcs.baidu.com/thumbnail/e16c5332d8ee95825fcdf5508288093e?fid=2402591948-250528-583386680802265&time=1431874800&rt=sh&sign=FDTAER-DCb740ccc5511e5e8fedcff06b081203-BDfd%2Fc%2Br805fRHpAsrhnnuAixxg%3D&expires=2h&prisign=unkown&chkv=0&chkbd=0&chkpc=&size=c850_u580&quality=100)
 
 
**需要测试的功能大致有:**   

1. 触摸按键  
2. CH340
3. 三轴加速度计/需要串口输出吧.
4. 蜂鸣器 
5. 模拟输出
6. PMW输出
7. SD卡
8. RTC
9. mini PCI-e(需要示波器)


**板卡测试参与成员:** 
 * 张进东
 * 侯名 
 * 解海超
 * 陈红林
 * 刘栋
    
 
