#pragma once

#define TESS_OCR_API

#ifdef __cplusplus
extern "C" {
#endif

enum {
    TESS_OCR_OK = 0,
    TESS_OCR_ERR_INVALID_ARGUMENT = -1,
    TESS_OCR_ERR_START_FAILED = -2,
    TESS_OCR_ERR_BUFFER_TOO_SMALL = -5,
    TESS_OCR_ERR_TIMEOUT = -6
};

// Runs the full webcam capture flow (opens webcam UI, waits for OCR final result).
// Default behavior is in-process capture (no webcam_ocr.exe required) when built with webcam support.
// To force process mode, set TESS_OCR_CAPTURE_USE_EXE=1 or pass webcam_ocr_exe_path.
// webcam_ocr_exe_path: optional process-mode path, defaults to TESS_OCR_WEBCAM_EXE env var or "<module_dir>/webcam_ocr.exe".
// timeout_ms: in in-process mode this must be 0; process mode supports finite timeout.
TESS_OCR_API int tess_ocr_capture_from_webcam(
    const char* webcam_ocr_exe_path,
    unsigned int timeout_ms,
    char* out_text,
    unsigned int out_text_capacity
);

#ifdef __cplusplus
}
#endif
