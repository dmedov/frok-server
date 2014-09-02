TEMPLATE = subdirs

FrokSolution.pro

SUBDIRS =               \
    FrokServerApp       \
    FrokAgentApp        \
    FrokAPILib          \
    FaceDetectionLib    \
    FaceRecognitionLib  \
    FrokLibCommon       \
    FrokTestApp         \
    FrokJsonlib         \

FrokServerApp.depends = FaceDetectionLib FaceRecognitionLib FrokLibCommon
FrokAgentApp.depends = FaceDetectionLib FaceRecognitionLib FrokLibCommon FrokAPILib
FrokTestApp.depends = FaceDetectionLib FaceRecognitionLib FrokLibCommon FrokAPILib

