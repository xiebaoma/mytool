#include "file_constant.h"
namespace spdb::sdk::file {

const char* file_error_str(FileErrorCode error_code) {
    switch (error_code) {
        case FileErrorCode::SUCCESS:
            return "Operation successful";
        case FileErrorCode::UNKNOWN_FILE_TYPE:
            return "Unknown file type";
        case FileErrorCode::OFFSET_OUT_OF_RANGE:
            return "Offset exceeds valid range";
        case FileErrorCode::NOT_ENOUGH_SPACE:
            return "Not enough space";
        case FileErrorCode::UNKNOWN_ERROR:
            return "Unknown error";
        case FileErrorCode::FILE_NOT_EXIST:
            return "File does not exist";
        case FileErrorCode::UNSUPPORTED_TYPE:
            return "Unsupported type";
        case FileErrorCode::DATAFILE_NOT_MATCH_PAGESIZE:
            return "Data file does not match page size";
        case FileErrorCode::DATAFILE_NOT_ALIGN_PAGESIZE:
            return "Data file not aligned with page size";
        case FileErrorCode::DATAFILE_ALLOC_NOT_MATCH:
            return "Data file allocation does not match";
        case FileErrorCode::DATAFILE_WRITE_MULTI_BLOCK:
            return "Data file write multiple blocks";
        case FileErrorCode::LOG_OR_DW_FILE_INVALID_FALLOC:
            return "Invalid file allocation for log or data write file";
        case FileErrorCode::META_DATA_OPEN_FAILED:
            return "Failed to open metadata";
        case FileErrorCode::META_DATA_DELETE_FAILED:
            return "Failed to delete metadata";
        case FileErrorCode::NOT_SUPPORT_RENAME:
            return "Rename operation not supported";
        case FileErrorCode::META_DATA_RENAME_FAILED:
            return "Failed to rename metadata";
        case FileErrorCode::META_ALLOC_BLOCK_FAILED:
            return "Failed to allocate metadata block";
        case FileErrorCode::READ_ONLY_FILE:
            return "File is read-only";
        default:
            return "Undefined error";
    }
} 

} // namespace spdb::sdk::file  