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

通过reshape得到的对象具有类似指针的语义，对其进行的访问会反映到原有对象上面，container_pointer.h中定义了array_ptr<type>(别名tensor_ptr<type,1>),tensor_ptr<type,n>,basic_string_ptr<type,char_traits>四种类指针对象，每种对象都有其相应的常值版本，其访问方法和标准库中的序列容器完全类似，拥有begin(),end(),operator[]等成员函数。

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
