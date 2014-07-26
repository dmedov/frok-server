TEMPLATE = subdirs

btmtest.pro

SUBDIRS =               \
    FaceServerApp       \
    FaceAgentApp        \
    FaceDetectionLib    \
    FaceRecognitionLib  \
    FaceCommonLib       \
    FrokAPILib          \

FaceServerApp.depends = FaceDetectionLib FaceRecognitionLib FaceCommonLib
FaceAgentApp.depends = FaceDetectionLib FaceRecognitionLib FaceCommonLib FrokAPILib

