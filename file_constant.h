#pragma once

#include <cstdint>

namespace spdb::sdk::file {

enum FileType {
    UNKNOWN = 0,       ///< Unknown file type
    REDO = 1,         ///< Redo log file
    DOUBLE_WRITE = 2, ///< Double write buffer file
    DATA = 3,         ///< Data file
    UNDO = 4          /// undo 是特殊的data文件
};

enum FileErrorCode {
    SUCCESS = 0,
    UNKNOWN_FILE_TYPE = 1,
    OFFSET_OUT_OF_RANGE = 10,
    NO_ACCESS = 11,
    NOT_ENOUGH_SPACE = 20,
    UNKNOWN_ERROR = 21,
    FILE_NOT_EXIST = 22,
    UNSUPPORTED_TYPE = 23,
    BAD_FD = 24,
    DATA_FILE_INVALID_SEGMENT_INDEX = 25,
    NO_MEMORY = 26,
    IO_ERROR = 27,
    INVALID_ARG = 28,

    NO_ENT = 29,
    NOT_DIR = 30,
    IS_DIR = 31,

    META_DATA_CRASH = 32,
    DIR_NOT_EMPTY = 33,
    FILE_EXIST = 34,
    NAME_TOO_LONG = 36,

    DATAFILE_NOT_MATCH_PAGESIZE = 100,
    DATAFILE_NOT_ALIGN_PAGESIZE = 101,
    DATAFILE_ALLOC_NOT_MATCH = 102,
    DATAFILE_WRITE_MULTI_BLOCK = 103,
    LOG_OR_DW_FILE_INVALID_FALLOC = 110,
    META_DATA_OPEN_FAILED = 200,
    META_DATA_DELETE_FAILED = 201,
    NOT_SUPPORT_RENAME = 202,
    META_DATA_RENAME_FAILED = 203,
    META_ALLOC_BLOCK_FAILED = 204,
    READ_ONLY_FILE = 205
};

const char* file_error_str(FileErrorCode error_code);


}

