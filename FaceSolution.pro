TEMPLATE = subdirs

btmtest.pro

SUBDIRS =               \
    FaceServerApp       \
    FaceDetectionLib    \
    FaceRecognitionLib  \
    FaceCommonLib       \

FaceDetectionApp.depends = FaceDetectionLib FaceRecognitionLib FaceCommonLib

