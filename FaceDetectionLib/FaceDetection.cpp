
cascades = new FaceCascades[MAX_THREADS_AND_CASCADES_NUM];
for(unsigned i = 0; i < MAX_THREADS_AND_CASCADES_NUM; i++)
{
    cascades[i].face->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml");
    cascades[i].eyes->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_eye_tree_eyeglasses.xml");
    cascades[i].righteye->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_righteye.xml");
    cascades[i].lefteye->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_lefteye.xml");
    cascades[i].righteye2->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_righteye_2splits.xml");
    cascades[i].lefteye2->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_lefteye_2splits.xml");
    cascades[i].eye->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_eye.xml");
    cascades[i].nose->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_nose.xml");
    cascades[i].mouth->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_mouth.xml");
}
