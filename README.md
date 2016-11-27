# How to run the program
1. swich to "105062551_HW1" directory 
2. $ _make clean_
3. $ _make_
4. $ _opt -load ./HW.so -HW case<mark>x</mark>.ll (case1~4)_

- You can test other cases by using :  
$ _clang -S -emit-llvm testname.c -o testname.ll_  
to generate the .ll file and test them.  

# Describe the cases that you can handle
#### i + c & c*i + d style :
	1. Output Dependency
	2. Flow Dependency
	3. Anti Dependency

	
#Example
####testcsae4 :  

![testcase4](http://imgur.com/kTHm8O6.png)

####output :  
![output](http://imgur.com/ssH9lrc.png)

####Analysis :
##### Output Dependency -
(i = 0, i = 1)  
D : S2 ---> S3  

i = 0 :  
A[0] = C[0];  
<mark>D[0]</mark> = A[-4];  
D[-1] = C[0];  
i = 1 :  
A[1] = C[1];  
D[1] = A[-1];  
<mark>D[0]</mark> = C[2];
        
---
##### Flow Dependency -  
(i = 2, i = 2)  
A : S1 ---> S2  

i = 2 :  
<mark>A[2]</mark> = C[2];  
D[2] = <mark>A[2]</mark>;  
D[1] = C[4];

---
##### Anti Dependency -  
(i = 3, i = 5)  
A : S2 ---> S1

i = 3 :  
A[3] = C[3];  
D[3] = <mark>A[5]</mark>;  
D[2] = C[6];  
i = 5 :  
<mark>A[5]</mark> = C[5];  
D[5] = A[11];  
D[4] = C[10];
