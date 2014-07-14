TEMPLATE = subdirs

btmtest.pro

SUBDIRS =               \
    FaceServerApp       \
    FaceAgentApp        \
    FaceDetectionLib    \
    FaceRecognitionLib  \
    FaceCommonLib       \

FaceDetectionApp.depends = FaceDetectionLib FaceRecognitionLib FaceCommonLib

