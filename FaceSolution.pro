TEMPLATE = subdirs

btmtest.pro

SUBDIRS =               \
    FrokServerApp       \
    FrokAgentApp        \
    FrokAPILib          \
    FaceDetectionLib    \
    FaceRecognitionLib  \
    FaceCommonLib       \

FrokServerApp.depends = FaceDetectionLib FaceRecognitionLib FaceCommonLib
FrokAgentApp.depends = FaceDetectionLib FaceRecognitionLib FaceCommonLib FrokAPILib

