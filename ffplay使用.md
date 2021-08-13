## ffplay使用

**播放本地文件**
　　ffplay -window_title "test time" -ss 2 -t 10 -autoexit test.mp4
　　ffplay buweishui.mp3

**播放网络流**
　　ffplay -window_title "rtmp stream" rtmp://202.69.69.180:443/webcast/bshdlive-pc

**强制解码器**
　　mpeg4 解码器： ffplay -vcodec mpeg4 test.mp4
　　h264 解码器： ffplay -vcodec h264 test.mp4

**禁用音频或视频**
　　禁用音频： ffplay test.mp4 -an
　　禁用视频： ffplay test.mp4 -vn

**播放 YUV 数据**
ffplay -pixel_format yuv420p -video_size 320x240 -framerate 5 yuv420p_320x240.yuv

ffplay -pixel_format nv12 -video_size 320x240 -framerate 5 NV12_320x240.yuv

**播放 RGB 数据**
ffplay -pixel_format rgb24 -video_size 320x240 -i rgb24_320x240.rgb

ffplay -pixel_format rgb24 -video_size 320x240 -framerate 5 -i rgb24_320x240.rgb
**播放 PCM 数据**
　　ffplay -ar 48000 -ac 2 -f f32le 48000_2_f32le.pcm
　　-ar set audio sampling rate (in Hz) (from 0 to INT_MAX) (default 0)
　　-ac set number of audio channels (from 0 to INT_MAX) (default 0)