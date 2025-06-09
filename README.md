![image](https://raw.githubusercontent.com/Li-Chaoran/Magnetic-coupled_Underwater_Vector_Propeller_Based_on_Hybrid_Mechanism/refs/heads/main/Pictures/cover.png)
# 基于混联机构的磁耦合水下矢量推进器
该库为本人2025年的毕业设计，使用数字孪生和解析解两种方式控制推进器。前者是在Unity平台配合ROS2完成，后者是基于STM32F407+FreeRTOS完成。<br/>
This library is my graduation project in 2025, using digital twin and analytical solution to control the propeller. The former is completed on Unity platform in cooperation with ROS2, while the latter is based on STM32F407+FreeRTOS.
## STM32F407 control pkg
此文件为STM32驱动的整体源文件，固件部署在魔女的F407VET6开发板上，如需购买请移步至淘宝。内容移植了CAN驱动，改进移植了达妙科技的驱动库，LCD屏驱动，使用USB协议的PS2手柄驱动，高精度定时器驱动，USART串口，W25Q128Flash驱动，XPT2046触摸屏驱动。V1.0固件只包含电机监控的控制，V2.0固件新增逆运动学的控制。如有问题请随时联系我，谢谢。<br/>
This file is the overall source file driven by STM32. Firmware is deployed on the development board F407the development board F407VET6. If you want to purchase it, please go to Taobao. The contents are transplanted with CAN drive, improved and transplanted with the drive library of Damiao Technology, LCD screen drive, USB protocol PS2 handle drive, high-precision timer drive, USART serial port, W25Q128Flash drive and XPT2046 touch screen drive. V1.0 firmware only includes the control of motor monitoring, V2.0 firmware adds the control of inverse kinematics. Please do not hesitate to contact me if you have any questions, thank you.
## Underwater vector propeller Digital twin
此文件是数字孪生APP，基于Unity开发，如有需要可以自行下载。没有添加过多的功能，可拓展性强。<br/>
This file is a digital twin APP developed based on Unity and can be downloaded if necessary. No too many functions added, strong scalability.
## Ansys simple check
此文件夹包含了使用Ansys2021对该推进器的关键承重零件的静态有限元分析和对齿轮啮合部分进行了有限元分析。在其中的result文件夹中存放了相关的结构，经过对ABS齿轮的有限元仿真，在电机3.5NM的额定扭矩，速度在1s转动半圈的情况下，齿轮副连接不会被破坏，当然这个条件在现实驱动中，显然是远远不成立的，所以该结构能可靠运行。<br/>
This folder contains the static finite element analysis of the key bearing parts of the propeller using Ansys2021 and the finite element analysis of the gear mesh section. Relevant structures are stored in the result folder. Through finite element simulation of ABS gear, the gear pair connection will not be damaged when the rated torque of the motor is 3.5NM and the speed is rotated for half a turn in 1s. Of course, this condition is obviously far from being established in the actual driving. Therefore, this structure can operate reliably.
## 其他文件的相关说明
如需知晓逆运动学的具体步骤，可查阅My Undergraduate Thesis文件，这是我的毕业论文。<br/>
如需知晓推进器的内部结构，可查阅Assembly Drawings文件，这是我的图纸。<br/>
如需查看具体演示视频，可移步至网址
