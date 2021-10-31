# ffmpeg-demo


## x264

源码地址：<https://code.videolan.org/videolan/x264.git>

编译：  
```
./configure --prefix=../../libx264 --disable-asm --enable-shared --enable-static

make -j 8

make install
```
&nbsp;

## ffmpeg

Github地址：<https://github.com/FFmpeg/FFmpeg>

编译：  
```
sudo apt install pkg-config

env PKG_CONFIG_PATH=../../libx264/lib/pkgconfig ./configure --prefix=../../ffmpeg --disable-x86asm --enable-shared --enable-gpl --enable-libx264 --disable-static

make -j 8

make install
```
&nbsp;