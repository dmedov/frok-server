
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


/*std::vector<std::string> users = std::vector<std::string>();
getSubdirsFromDir(ID_PATH, users);

if(users.empty())
{
    FilePrintMessage(_WARN("No trained users found in %s folder"), ID_PATH);
    return;
}

for (unsigned int i = 0; i < users.size(); i++)
{
    Ptr<FaceRecognizer> model = createFisherFaceRecognizer();
    try
    {
        string fileName = ((string)ID_PATH).append(users[i]).append("/eigenface.yml");
        if (access(fileName.c_str(), 0) != -1)
        {
            model->load(fileName.c_str());
            FilePrintMessage(_SUCC("Model base for user %s successfully loaded. Continue..."), users[i].c_str());
        }
        else
        {
            FilePrintMessage(_WARN("Failed to load model base for user %s. Continue..."), users[i].c_str());
            continue;
        }

    }
    catch (...)
    {
        FilePrintMessage(_WARN("Failed to load model base for user %s. Continue..."), users[i].c_str());
        continue;
    }
    models[users[i]] = model;
}*/


/*delete []cascades;
for (int i = 0; i < MAX_THREADS_AND_CASCADES_NUM; i++)
{
    cvReleaseHaarClassifierCascade(&cascades[i].face->oldCascade);
    cvReleaseHaarClassifierCascade(&cascades[i].eyes->oldCascade);
    cvReleaseHaarClassifierCascade(&cascades[i].nose->oldCascade);
    cvReleaseHaarClassifierCascade(&cascades[i].mouth->oldCascade);
    cvReleaseHaarClassifierCascade(&cascades[i].eye->oldCascade);
    cvReleaseHaarClassifierCascade(&cascades[i].righteye2->oldCascade);
    cvReleaseHaarClassifierCascade(&cascades[i].lefteye2->oldCascade);
}*/

