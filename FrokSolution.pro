TEMPLATE = subdirs

FrokSolution.pro

SUBDIRS =               \
    FrokAgentApp        \
    FrokAPILib          \
    FaceDetectionLib    \
    FaceRecognitionLib  \
    FrokLibCommon       \
    FrokTestApp         \
    FrokJsonlib         \

FrokAgentApp.depends = FaceDetectionLib FaceRecognitionLib FrokLibCommon FrokAPILib
FrokTestApp.depends = FaceDetectionLib FaceRecognitionLib FrokLibCommon FrokAPILib

