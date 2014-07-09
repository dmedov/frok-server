TEMPLATE = subdirs

btmtest.pro

SUBDIRS =               \
    FaceDetectionApp    \
    FaceDetectionLib    \

FaceDetectionApp.depends = FaceDetectionLib
