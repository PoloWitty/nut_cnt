# 螺母计数

> HUSTAIA 2022 FALL

 

## 结果

原图，效果图：

<img src="README.assets\test.bmp" alt="test" style="zoom: 33%;" /><img src="README.assets\image-20221005124731702.png" style="zoom: 25%;" />



控制台结果输出（ $\gamma=141,e=6,T_d=1500,T_u=4200$ ）：

```bash
ours sibling:15
315 346 829 1923 1933 1952 1999 2046 2895 2983 2984 3003 3054 3139 3200
nut num: 12
```



## 算法

算法流程：

1. 以阈值 $\gamma$ 二值化
2. 以 $e$ 为核大小进行腐蚀
3. 寻找图像中所有的内轮廓并计算面积
4. 依据下阈值 $T_d$ 和上阈值 $T_u$ 筛选内轮廓面积进行计数

注：

- 实验过程中注意细致调整相机光圈，焦距，底座亮度，二值化阈值，腐蚀核大小，面积下阈值和上阈值，这些对实验结果都会造成影响
  - 相机视域内不要出现底座边框
  - 调整焦距光圈使成像清晰
  - 二值化阈值默认值可以设为150左右
  - 腐蚀核大小不要过小，默认值可以设为6左右
  - 面积下阈值和上阈值依据控制台输出的当前实验条件下的所有内轮廓面积大小进行选取（不同相机距离、腐蚀核大小等条件下螺母内轮廓大小并不一样）
- 基本就是调参，参数并不是很好调，可能需要消耗些时间
- 在结果图像中将以灰色显示出螺母内轮廓
- 如果可以的话，尽可能减少粘连，比较强的粘连实在无法处理



## 实现细节

这个只是一个简单的调用了opencv的测试demo，后续还需自行整合

countour函数改自[FreshJesh5/Suzuki-Algorithm (github.com)](https://github.com/FreshJesh5/Suzuki-Algorithm)，具体见coutour.cpp



简易版腐蚀（可以用在自己的代码中）：

```cpp
/*
param:
 nut_bImage
 k: kernel size
author: polowitty
*/
void erode(MVImage& nut_bImage, int k)
{
	int num_row = nut_bImage.GetHeight();
	int num_col = nut_bImage.GetWidth();
	UCHAR* b_p = (UCHAR*)nut_bImage.GetBits();//获取二值图内存地址
	UCHAR minV;
	UCHAR pixel_v;
	for (int i = 0; i < num_row; i++)
	{
		for (int j = 0; j < num_col; j++)
		{
			minV = 255;
			for (int xi = i; xi <= i + k; xi++)
			{
				for (int yi = j; yi <= j + k; yi++)
				{
					if (xi<0 || xi>=num_row || yi < 0 || yi >= num_col) continue;

					pixel_v = *(b_p + xi * num_col + yi);
					minV = min(minV, pixel_v);
				}
			}
			*(b_p + i * num_col + j) = minV;
		}
	}
}
```





