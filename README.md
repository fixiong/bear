# bear
将任意连续内存对象抽象成多位数组的C++库
<br><br>


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

* array_ptr<type> （别名tensor_ptr<type,1>）
* tensor_ptr<type,n>
* basic_string_ptr<type,char_traits> （array_ptr的特化版本，支持部分std::string的对应方法）
* image_ptr<type,channel> （tensor_ptr<type,2>的特化版本，用于支持一些图像操作，于"image.h"中定义）
	
	

#### 多维容器

tensor.h中定义了持有自己内存空间的多维容器，可以直接创建，也可以通过vector组合一个类指针得到，本身也可以拆解成指针和vector：

```c++
	//直接创建
	tensor<int, 3> t1(3,4,5);

	//reshap一个临时vector得到
	tensor<int, 3> t2 = reshape(vector<int>(60, 0), 3, 4, 5);

	//拆解为vector
	std::pair<std::vector<int>, tensor_ptr<int, 3>> pr = t1.decompose();
```

image<type,channel>与其类似，对应的类指针对象为image_ptr<type,channel>，在"image.h"中定义。

<br><br><br>

## 常用的方法

库中定义有一些常用的处理多维对象的方法。

### * clip(ptr,start,end)
### * clip_at<n>(ptr,start,end)

两个函数同为对类指针对象进行切片的函数，取得序号[start, end)范围的元素，第二个定义支持对指定的维度切片。
切片操作不会进行内存拷贝，但会使生成的指针指向不连续的内存空间。

范列：

```c++
	tensor<int,2> img(20,20);
	clip_at<1>(img,0,10).fill(1); //将容器第二个维度前一半的元素设为1
```
<br><br>

### * reshape(ptr,sizes...)

取得改变维度及各维度尺寸的类指针的函数，要求变形后与变形前对象包括的元素总数目一致，不一致会抛出异常，另外如果要做变形处理的内存区域不连续（比如执行clip()后取得的对象）并且元素的内存分布不满足需要变化到的尺寸的连续性要求的话，reshape()有失败的可能，失败时返回回空对象。

reshape存在一个接受vector右值的版本，利用vector原有的内存空间构建一个tenser容器。

<br><br>

### * tensor_ptr::plan()

取得一个1维类指针，指向对象原有的内存空间，要求原有内存空间完全连续，否则返回空类指针对象。

范列：

```c++
	tensor<int,3> cube(20,20,20);
	tensor_ptr<int,1> ary = cube.plan();
	std::for_each(ary.begin(),ary.end(),[]( int i ) { std::cout << i; }); //顺序遍历cube中所有元素
```
<br><br>

### * shrink(ptr)

对对象进行变形，最大限度缩减尺寸不为1的维度的数目，并将尺寸不为1的维度集中到最后，和reshape不同，此函数一定成功，在遍历数组前使用此函数可以加快迭代速度。

范列：

```c++
	tensor<int,3> cube(20,20,20);
	tensor_ptr<int,3> p = shrink(cube); //p的尺寸为（1 ，1 ，8000）
```
<br><br>

### * size(ptr)

返回对象的尺寸，对于超过1维的对象，size()返回一个std::array<size_t,dim>对象，其中dim为维数，对image_ptr，则返回包涵长宽的结构体。

范列：

```c++
	tensor<int,3> cube(20,20,20);
	std::array<std::size_t,3> sz = size(cube);
	tensor<float,3> cube_float(sz); //创建一个与前一容器尺寸相同的容器。
```
<br><br>

### * to_ptr(container)

取得指向支持的容器的类指针对象，作用到指针对象的话，则返回指针对象本身的引用。

范列：

```c++
	std::vector<int> v{1,2,3,4,5};
	to_ptr(v).for_each([](int i){std::cout << i; }); //使用相应的指针对象遍历vector
```
<br><br>

### * tensor_ptr::fill(element)
### * tensor_ptr::for_each(functor)

如之前的范列所示，这两个函数对每个元素进行填充数值或其他相应的操作。

<br><br><br>



## 进阶函数

"ptr_algorism.h"中定义有一些更复杂的操作数组的函数。

### * zip_to<n>(functor,ptr ...)

传入一个函数对象，和一系列多维数组对象，遍历前n个维度，将每个数组指定维度的每一元素做为参数调用函数对象，要求数组的前n个维度尺寸一致。（只要是可以多维迭代的对象都可以传入，不一定是多维数组）

范列：


```c++
	std::list<std::array<int, 10>> lst(20);
	tensor<char, 3> ts = reshape(vector<char>(1000,100),20, 10, 5);

	zip_to<2>([](int &lv, tensor_ptr<char, 1> tv)
	{
		lv = sum(tv); //对ts的最后一维求总和，结果保存在lst中

	}, lst, ts); //lst 中的所有元素赋值为 500
```

<br><br>

### * map_function(functor,ptr ...)

传入一个以后面参数元素类型作为参数的函数对象，遍历每个元素，调用传入的函数对象，结果（如果返回不为void）存为一个同样尺寸的数组，参数需要是支持的连续内存序列容器或类指针，要求参数尺寸一致。

范列：


```c++
	auto fun = [](int i, double f)
	{
		return i / f;
	};

	tensor<int, 2> ts_int = reshape(std::vector<int>(100, 10), 20, 5);
	tensor<double, 2> ts_double = reshape(std::vector<double>(100, 1.5), 20, 5);

	tensor<double, 2> rst = map_function(fun, ts_int, ts_double);
```

<br><br>

### * compare(arr1,arr2)

按字符串顺序比较两个多维数组，返回-1，0，1，类似strcmp，不要求数组的尺寸相同，但需要数组的维数一致，可以用来对任意形状的数组进行排序。

<br><br>

### * for_each(ptr,fun)
### * fill(ptr,value)

和成员函数版本的对应函数功能相同，增加了对std::array<type,size>的支持。

范列：

```c++
	tensor<std::array<int, 5>, 2> ts(10,10);

	fill(ts, 50); //将ts中每个std::array的每个元素设为50
	to_ptr(ts).fill(std::array<int, 5>{1, 2, 3, 4, 5}); //将ts中每个std::array设为1,2,3,4,5
```

<br><br>

### * split(const_string_ptr,token) （basic_string_ptr独有）

按分割符拆分字符串，返回装有指向字符串每个分隔部分的类指针vector（std::vector<const_string_ptr>类型），整个操作不需要拷贝字符串。


<br><br><br>

### 数值函数

"ptr_numeric.h"中定义有一些对数组进行数值操作的函数

##### 重定义操作符：

重定义了+,-,* 等数值操作符号，可以直接作用到数组上面，包括数组与数组，数组与标量的四则运算，%运算，| & ^位运算，及其对应的带等号赋值版本。

数组与数组之间的运算要求维度和尺寸一致。

范列：

```c++
	tensor<float, 2> img1(50, 50);
	tensor<float, 2> img2(50, 50);
	tensor<float, 2> alpha(50, 50);
	
	.......
	
	img1 *= alpha;

	tensor<float, 2> img3 = img1 + img2 * (1.0f - alpha); //按alpha混合img1与img2
```

<br><br>

### * dot(ptr,ptr)

对一维或二维对象进行点乘运算的函数，一维对象（即内积）要求尺寸一致，二维对象（即矩阵乘法）要求前一操作数的第一个维度与后一操作数的第二个维度尺寸一致，不一致的情况会抛出异常。

<br><br>

### * element_case<type>(ptr)

对数组中每一个元素调用static_cast<type>，结果保存在新的数组中。
	
范列：

```c++
	tensor<int, 2> ts_int(10, 10);

	tensor<double, 2> ts_double= element_cast<double>(ts_int);
```


<br><br><br>

### 其他计划中的部分

包括tensor,image的内存对齐版本aligned_tensor,aligned_image,进行卷积运算的conv_1d(),conv2d()函数，对数组某一维求和，方差等的统计函数，正在编写中


