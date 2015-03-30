# Source
This is an extension of Niu Zhiheng's [work](https://github.com/niuzhiheng/caffe) on Caffe Windows installation.

# Windows Installation
This is not the original [Caffe Readme](https://github.com/BVLC/caffe/blob/master/README.md) but an installation guide for windows version.

#### Want to run first before build by yourself?
You can download the windows x64 [standalone package](https://dl.dropboxusercontent.com/u/3466743/caffe-vs2012/standalone.7z) and run directly on MNIST dataset.

#### Prerequisites
You may need the followings to build the code:
- Windows 64-bit
- [Boost] (http://sourceforge.net/projects/boost/files/boost-binaries/) 
- [cuDNN] (https://developer.nvidia.com/cuDNN)
- MS Visual Studio 2013
- CUDA toolkit 7.0
- Other dependencies which you can directly download from [here](https://dl.dropboxusercontent.com/u/1308750/3rdparty.zip).
- BOOST_ROOT environment variable to point at the Boost installation directory
- CUDNN_ROOT environment variable to point at the cuDNN installation directory

#### Build Steps
Currently it can be built by VS2013 for x64 platform only. This is because the dependencies mentioned above is cross-compiled to support x64 only. If you want to build on 32bit windows, you need to rebuild your own 3rd-party libraries.
- Check out the code 
- Download the [dependency file](https://dl.dropboxusercontent.com/u/1308750/3rdparty.zip) and extract the folders inside to project root directory.
- Include any .cpp you want to build in the `./tools` directory in the project build
- Open the solution file in `./build/MSVC`
- Switch build target to x64 platform Release.
- Build the code and you may find the `./bin/MainCaller.exe`

#### Validating the Build on MNIST dataset
- Suppose you choose to build train_net.cpp which is the default one in MainCaller.cpp
- If you do not have GPU, please change it to CPU in `lenet_solver.prototxt`
- Start command prompt in `./examples/mnist`
- Run `get_mnist_leveldb.bat` to download the dataset in leveldb format.(ignore)
- Run `train_lenet.bat` to see the training progress .

#### Tips
- It takes obvious longer time when you compile for the first time. Therefore please refrain from using `clean & rebuild`.
- To support different [GPU compute capabilities](http://en.wikipedia.org/wiki/CUDA#Supported_GPUs), the code is built for several compute capability versions. If you know the exact version of your GPU device, you may remove the support to other versions to speed up the compiling procedure. You may wish to take a look at #25 for more details.

#### Known Issues
- ~~I have trained on ImageNet with this windows porting as well. The speed is much slower than the one built on Ubuntu. 20 iterations take 79s on Windows, whereas same number of iterations take about 30s on Ubuntu (on GTX Titan).~~
- The above issue has been solved since the upgrade of GPU driver to 340.62 and CUDA to 6.5. The same hardware is now running 26 sencods for 20 iterations on Windows. Hooray!!

#### Bug Report
- Please create new issues in [github](https://github.com/niuzhiheng/caffe/issues) if you find any bug.
- If you have new pull requests, they are very welcome.
- Before you do that, you may wish to read this [wiki page](https://github.com/niuzhiheng/caffe/wiki) for more information.


Please let me know if you found issues.

Thanks @happynear and NIU ZHIHENG (https://github.com/niuzhiheng/caffe) for their contributions to caffe on windows.
