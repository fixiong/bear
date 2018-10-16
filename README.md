# bear
将任意连续内存对象抽象成多位数组的C++库



## 基本用法

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

* array_ptr<type> （tensor_ptr<type,1>别名）
* tensor_ptr<type,n>
* basic_string_ptr<type,char_traits> （array_ptr的特化版本，支持部分std::string方法）
* image_ptr<type,channel> （tensor_ptr<type,2>的特化版本，用于支持一些图像操作，于"image.h"中定义）
	
	

## 多维容器

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



## 常用的方法

库中定义有一些常用的处理多维对象的方法：

### * clip(ptr,start,end)
### * clip_at<n>(ptr,start,end)

两个函数同为对类指针对象进行切片的函数，取得序号[start, end)范围的元素，第二个定义支持对指定的维度切片。
切片操作不会进行内存拷贝，但会使生成的指针指向不连续的内存空间。

范列：

```c++
	tensor<int,2> img(20,20);
	clip_at<1>(img,0,10).fill(1); //将容器第二个维度前一半的元素设为1
```


### * reshape(ptr,sizes...)

取得改变维度及各维度尺寸的类指针的函数，要求变形后与变形前对象包括的元素总数目一致，不一致会抛出异常，另外如果要做变形处理的内存区域不连续的话（比如执行clip()后取得的对象），reshape()有失败的可能，元素的内存分布不满足需要变化到的尺寸的连续性要求的话，reshape()会返回空对象。

reshape存在一个接受vector右值的版本，利用vector原有的内存空间构建一个tenser容器。



### * tensor_ptr::plan()

取得一个1维类指针，指向对象原有的内存空间，要求原有内存空间完全连续，否则返回空类指针对象。

范列：

```c++
	tensor<int,3> cube(20,20,20);
	tensor_ptr<int,1> ary = cube.plan();
	std::for_each(ary.begin(),ary.end(),[]( int i ) { std::cout << i; }); //顺序遍历cube中所有元素
```


### * shrink(ptr)

对对象进行变形，最大限度缩减尺寸不为1的维度的数目，并将尺寸不为1的维度集中到最后，和reshape不同，此函数一定成功，在遍历数组前使用此函数可以加快迭代速度。

范列：

```c++
	tensor<int,3> cube(20,20,20);
	tensor_ptr<int,3> p = shrink(cube); //p的尺寸为（1 ，1 ，8000）
```


### * size(ptr)

返回对象的尺寸，对于超过1维的对象，size()返回一个std::array<size_t,dim>对象，其中dim为维数，对image_ptr，则返回包涵长宽的结构体。

范列：

```c++
	tensor<int,3> cube(20,20,20);
	std::array<std::size_t,3> sz = size(cube);
	tensor<float,3> cube_float(sz); //创建一个与前一容器尺寸相同的容器。
```


### * to_ptr(container)

构建指向支持的容器的类指针对象，作用到指针对象的话，则返回指针本身的引用。

范列：

```c++
	std::vector<int> v{1,2,3,4,5};
	to_ptr(v).for_each([](int i){std::cout << i; });
```


### * tensor_ptr::fill(element)
### * tensor_ptr::for_each(functor)

如之前的范列所示，这两个函数对每个元素进行填充数值或其他相应的操作。










