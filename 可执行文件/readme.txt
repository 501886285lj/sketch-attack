/*******************************************************************************
*			该软件由
*	华南理工大学 计算机科学与工程学院 网络空间安全 梁剑
*		      E-mail：doc.liang@qq.com
*		所		        	  有
******************************************************************************/
			目录介绍

attack.exe					执行文件，进行攻击
decode.cfg				解码的配置文件，一般不做修改
opencv_core2410.dll				opencv动态链接库
opencv_highgui2410.dll			opencv动态链接库
opencv_imgproc2410.dll			opencv动态链接库
test.264					测试文件，若没有对配置文件进行修改或输入-p InputFile则默认该文件进行输入
xxx_sketchPic				存放 攻击图片/canny图片 的文件夹
xxx_ESS.csv				输出框架攻击的量化指标ESS(若有yuv文件输入)


			使用方法

查看帮助文档				-h
H.264/AVC文件输入				-p InputFile="FileRoute"
fps					-fps
调整输出画面尺寸(超分)			-sr
计算ESS					-ESS xxx.yuv 
设置canny检测器的参数			-canny minThreshold maxThreshold size


			执行结果
· 执行该程序后会在同一路径下生成xxx_sketchPic文件夹，里面存放有框架攻击得到的图片
· 若使用了-ESS功能（注意yuv文件必须与攻击的文件相匹配），则在同一路径下会生成xxx_ESS.csv文件，该文件内部有ESS量化指标
