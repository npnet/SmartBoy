### 傅里叶变换
###inline:
`` 
    在C语言中，如果一些函数被频繁调用，不断地有函数入栈，即函数栈，会造成栈空间或栈内存的大量消耗。
 为了解决这个问题，特别的引入了inline修饰符，表示为内联函数。
 栈空间就是指放置程式的局部数据也就是函数内数据的内存空间，在系统下，栈空间是有限的，假如频繁大量的使用就会造成因栈空间不足所造成的程式出错的问题，函数的死循环递归调用的最终结果就是导致栈内存空间枯竭。
``

`
 写入：ioctl(pcm->fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES, &x)
 读：ioctl(pcm->fd, SNDRV_PCM_IOCTL_READI_FRAMES, &x)
 参数:ioctl(fd, SNDRV_PCM_IOCTL_HW_REFINE, params)
`

### 音频数据中的几个重要概念：

````
1.Sample：样本长度，音频数据最基本的单位，常见的有 8 位和 16 位；
2.Channel：声道数，分为单声道 mono 和立体声 stereo；
3.Frame：帧，构成一个完整的声音单元，所谓的声音单元是指一个采样样本，Frame = Sample * channel；
4.Rate：又称 sample rate，采样率，即每秒的采样次数，针对帧而言；
5.Period Size：周期，每次硬件中断处理音频数据的帧数，对于音频设备的数据读写，以此为单位；
6.Buffer Size：数据缓冲区大小，这里指 runtime 的 buffer size，而不是结构图 snd_pcm_hardware 中定义的 buffer_bytes_max；一般来说 buffer_size = period_size * period_count， period_count 相当于处理完一个 buffer 数据所需的硬件中断次数。
