# bear
将任意连续内存对象抽象成多位数组的C++库

### 基本用法

container_pointer.h中定义了很多基本类型，用于将连续的内存抽象为多维数组，其中最重要的函数之一是reshape()，用于重新定义对象的维数及尺寸，如下代码，将一个vector对象抽象为3维数组来对其进行访问：

```c++

#include "container_pointer.h"

using namespace bear;

int main()
{
	std::vector<int> mem_spc(60);

	tensor_ptr<int, 3> t_3d = reshape(mem_spc, 3, 4, 5);

	for (int z = 0; z < 3; ++z)
	{
		for (int y = 0; y < 4; ++y)
		{
			for (int x = 0; x < 5; ++x)
			{
				t_3d[z][y][x] = z * 100 + y * 10 + x;
			}
		}
	}

	std::cout << t_3d << endl;
}

```

通过reshape得到的对象具有类似指针及引用的语义，对其进行的访问会反映到原有对象上面，类指针对象拥有begin(),end(),operator[]等成员函数，各自分为普通和常值两个版本。

基础的类指针类型包括：

* array_ptr<type> （tensor_ptr<type,1>）
* tensor_ptr<type,n>
* basic_string_ptr<type,char_traits> （array_ptr的特化版本，支持部分std::string方法）
* image_ptr<type,channel> （tensor_ptr<type,2>的特化版本，用于支持一些图像操作，于"image.h"中定义）
	

### 多维容器

tensor.h中定义了持有自己内存空间的多维容器，可以直接创建，也可以通过vector组合一个类指针得到，本身也可以拆解成指针和vector：

```c++
	//直接创建
	tensor<int, 3> t1(3,4,5);

	//reshap一个临时vector得到
	tensor<int, 3> t2 = reshape(vector<int>(60, 0), 3, 4, 5);

	//拆解为vector
	std::pair<std::vector<int>, tensor_ptr<int, 3>> pr = t1.decompose();
```

image<type,channel>与其类似，对应的类指针对象为image_ptr<type,channel>

### 常用的方法

库中定义有一些常用的处理多维对象的方法：

* clip(ptr,start,end)
* clip_at<n>(ptr,start,end)

两个函数同为对类指针对象进行切片的函数，取得序号[start, end)范围的元素，第二个定义支持对指定的维度切片。
切片操作不会，进行内存拷贝，但会使生成的指针指向不连续的内存空间。

* reshape(ptr,sizes...)

取得改变维度及各维度尺寸的类指针对象的函数，要求变形后与变形前对象包括的元素总数目一致，不一致会抛出异常，另外如果要做变形处理的内存区域不连续的话（比如执行clip()后取得的对象），reshape()有失败的可能，元素的内存分布不满足需要变化到的尺寸的连续性要求的话，reshape()会返回空对象。

reshape存在一个接受vector右值的版本，利用vector原有的内存空间构建一个tenser容器。





