TEMPLATE = subdirs

FaceSolution.pro

SUBDIRS =               \
    FrokServerApp       \
    FrokAgentApp        \
    FrokAPILib          \
    FaceDetectionLib    \
    FaceRecognitionLib  \
    FaceCommonLib       \
    FrokTestApp         \

FrokServerApp.depends = FaceDetectionLib FaceRecognitionLib FaceCommonLib
FrokAgentApp.depends = FaceDetectionLib FaceRecognitionLib FaceCommonLib FrokAPILib
FrokTestApp.depends = FaceDetectionLib FaceRecognitionLib FaceCommonLib FrokAPILib

