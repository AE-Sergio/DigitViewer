/* yc_YCDWriter_internals.h - .ycd Writer Object
 * 
 * Author           : Alexander J. Yee
 * Date Created     : 07/29/2013
 * Last Modified    : 07/29/2013
 * 
 */

#pragma once
#ifndef _yc_YCDWriter_internals_H
#define _yc_YCDWriter_internals_H
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Dependencies
#include "yc_YCDFileWriter_headers.h"
#include "yc_YCDWriter_headers.h"
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
namespace DigitViewer{
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Helpers
void YCDWriter::create_file(ym_uL fileid){
    std::wstring full_path(path);
    full_path += name;
    full_path += L" - ";
    full_path += std::to_wstring(fileid);
    full_path += L".ycd";

    file = YCDFileWriter(full_path,first_digits,digits_per_file,fileid,radix);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
YCDWriter::YCDWriter(
    std::wstring path_,
    std::wstring name_,
    std::string first_digits,
    ym_uL digits_per_file_,
    ym_uL start_fileid,
    int radix_,
    ym_pL buffer_size,
    ym_u64 *buffer,
    void (*deallocator)(void*)
)
    : path(std::move(path_))
    , name(std::move(name_))
    , radix(radix_)
    , first_digits(std::move(first_digits))
    , digits_per_file(digits_per_file_)
    , fileid(start_fileid)
    , fp_free(deallocator)
{
    if (buffer_size < 4096)
        throw ym_exception("Requested buffer size is too small.",YCR_DIO_ERROR_INVALID_PARAMETERS);

    switch (radix){
        case 16: break;
        case 10: break;
        default:
            throw ym_exception("Unsupported Radix",YCR_DIO_ERROR_INVALID_BASE);
    }

    //  Make sure path ends in a slash.
    if (path.size() != 0){
        wchar_t back = path.back();
        if (back != '/' && back != '\\')
            path += '/';
    }

    //  Multiple files, create a folder for them.
    if (digits_per_file != (ym_uL)0 - 1){
        path += name;
        path += '/';
        ym_file_makedir(path.c_str());
    }

    bin_buffer_L = buffer_size / sizeof(ym_u64);
    if (buffer == NULL){
        external_buffer = false;
        bin_buffer = (ym_u64*)ym_ah_malloc(NULL,bin_buffer_L * sizeof(ym_u64),2*sizeof(ym_u64));
    }else{
        external_buffer = true;
        bin_buffer = buffer;
    }
}
YCDWriter::~YCDWriter(){
    //  Internally allocated.
    if (!external_buffer)
        ym_ah_free(bin_buffer);

    //  Preallocated with manual deallocator.
    if (fp_free != NULL)
        fp_free(bin_buffer);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void YCDWriter::write(char *str,ym_pL digits){
    while (true){
        //  No file is opened.
        if (!file.isValid()){
            create_file(fileid++);
        }

        //  Write digits
        ym_pL written = file.write_chars(str,digits,bin_buffer,bin_buffer_L);
        if (written == digits)
            return;

        digits -= written;
        str    += written;
    }
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
}
#endif
