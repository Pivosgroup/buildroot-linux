#################################################
#                                               #
#              IF_MANAGER模块说明               #
#                Version 1.1.0                  #
#                                               #
#################################################

1. 准备工作:
   1. 以太网:不需特殊编译
   
   2. wifi:在编译kernel时, 执行make k.menuconfig, 进行如下配置:
               在Networking-->Wireless中, 确保Generic IEEE 802.11 Networking Stack(有两个)都被选中
               在Device drivers-->Nerwork device support-->Wireless LAN中, 确保ZyDAS ZD1211/ZD1211B USB-wireless support被选为M
               执行make k.modules，就能在编译目录下的rootfs/lib/modules/中看到zd1211rw.ko(此为我们板子目前支持的一款wifi dongle的驱动)
               执行make p.RTL819X_Linux2.6_Driver.all，就能在编译目录下的rootfs/lib/modules/中看到8712u.ko（此为我们支持的另一款wifi dongle的驱动）
               将这两个ko文件放到项目使用的rootfs中，可以使用insmod命令动态加载        
               在编译package时, 执行make p.menuconfig, 进行如下配置:
               在Network Utils Packages Configure中, 确保wpa_supplicant_wps_patch support和Wrieless tools 29被选中
               在Basic Library Configure中, 确保openssl-0.9.8 support被选中
               
   3. pppoe:在编译busybox时, 执行make b.menuconfig, 进行如下配置: 
           在coreutils中, 确保basename, tty, uname三项被选中
           请将源码包中的src/scripts/ppp文件夹放在项目使用的rootfs/etc/下
           请将源码包中的src/bin/pppd, pppoe, pppoe-connect, pppoe-init, pppoe-relay, pppoe-server, pppoe-setup, pppoe-sniff, pppoe-start, pppoe-status, pppoe-stop放在项目使用的rootfs/bin/下

2. 编译说明:      
   1. 执行make p.menuconfig既可以在Network Utils Packages Configure中找到if_manager的选项，选中即可经make p编入rootfs中
      也可以手动编译，执行make p.if_manager-1.1.0.all, 即可将if_manager模块编入rootfs中
   2. 编译完成后, 可以在rootfs/lib/中找到相关库, 在rootfs/include/ifm/中找到头文件ifm_def.h和if_manager.h，在trunk/build/packages/if_manager-1.1.0/test/中找到测试程序ifm_test

3. 使用说明:
   1. 为双系统切换而使用此模块，请包含头文件ifm_def.h, 此接口文件提供了一个函数setNetwork(), 返回一个整数, 表示操作结果, 所有的返回值均在此头文件中以宏的形式表示, 便于查看失败原因.
   network_setting.c中, 有一个宏开关(USE_NAND), 默认关闭, 此宏表示是否需要设置的数据要从nand读取, 如果此宏被定义, 则需要另外的nand读取模块支持, 需要设置的数据会从nand读出来; 否则在setNetwork()中已经写入了几组测试数据,使用者也可自己按照格式输入测试内容. 正式发布时, 此宏需被定义, 同时需增加nand读取模块.
   
   2. 需使用管理模块, 请包含头文件if_manager.h, 具体可使用的API请参见if_manager.h. 一些返回值同样在ifm_def.h(此头文件已被if_manager.h包含，使用时可不必包含此文件)中查找相应的出错原因的宏定义值. 使用时请参看源码包中的src/test/ifm_test.c的实现，尤其是while(1)之前的调用顺序。后续会不断完善, 为if_manager.h增加新的接口.
   
   3. debug打印: 在ifm_def.h中打开//#define DEBUG_PRINT, 即可打开打印.
   
4. 功能说明：
   1. if_manager:
         实现了:以太网, wifi的动态和静态设置
                AP列表的搜索和结果获取
                pppoe的设置和连接, 断开功能
                查询某网络接口是否已经准备好（UP状态）
                设置某网络接口的启动和关闭
                查询某网络接口的详细信息
                查询整个网络的状态
                双系统方案中将AVOS的网络参数写入linux
                在用户关心的某接口状态发生变化的时候通过用户注册的回调函数通知用户
   2. ifm_test:
         实现了:初始化if_manager, 注册监视状态改变的回调函数, 启动daemon进程(使用时这个顺序不能错)
                查询以太网, wifi, pppoe的网口状态
                搜索AP, 打印AP搜索结果, 连接AP: AML_BJ
                将已获得IP地址的网口的ip信息打印出来
                                
        
