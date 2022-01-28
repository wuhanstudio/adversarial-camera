# ./app.py | ffmpeg -s 640x480 -f rawvideo -pix_fmt bgr24 -framerate 10 -i - -s 1280x720 -vf crop=w=640:h=360 -r 10 -f v4l2 -vcodec mjpeg -pix_fmt yuvj420p /dev/video0
./app.py -c 2 | ffmpeg -s 640x480 -f rawvideo -pix_fmt bgr24 -framerate 10 -i - -s 1280x720 -vf crop=w=640:h=360 -r 10 -f v4l2 -vcodec rawvideo -pix_fmt yuyv422 /dev/video0

# gst-launch-1.0 videotestsrc ! v4l2sink device=/dev/video0
# sudo ffmpeg -f v4l2 -input_format yuyv422 -i /dev/video40  -c:v mjpeg  -f mjpeg - > /dev/video42
# gst-launch-1.0 videotestsrc ! "video/x-raw, width=640, height=360, fps=30/1" ! avenc_mjpeg !  v4l2sink device=/dev/video42
