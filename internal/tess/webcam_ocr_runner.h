#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Runs the full webcam OCR flow.
// Args are the same as the webcam_ocr executable CLI args.
int tess_webcam_ocr_run(int argc, char** argv);

#ifdef __cplusplus
}
#endif

